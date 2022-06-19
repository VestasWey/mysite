#include "crash_handler_client.h"

#include <fstream>

#include <Windows.h>
#include <Psapi.h>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
//#include "base/process/kill.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"

#include "client/windows/handler/exception_handler.h"

#include "common/app_context.h"
#include "common/app_constants.h"
#include "common/app_logging.h"
#include "common/app_paths.h"
#include "common/app_crash_helper.h"

namespace {

    base::FilePath g_dump_dir;// %localappdata%lcpfw/User Data/Crash Reports

    MINIDUMP_TYPE DetermineDumpFlags()
    {
        auto flags = MiniDumpWithProcessThreadData |
            MiniDumpWithThreadInfo |
            MiniDumpWithUnloadedModules |
            MiniDumpWithIndirectlyReferencedMemory;

        if (base::CommandLine::ForCurrentProcess()->HasSwitch(lcpfw::kSwitchFullMinidump))
        {
            LOG(INFO) << "Full minidump mode is enabled!";
            flags |= MiniDumpWithFullMemory;
        }

        return static_cast<MINIDUMP_TYPE>(flags);
    }

    // Returns crash-id（in the form:"module_name+addr_offset"）on success, Returns empty string otherwise.
    // This is useful for flag crash event so that we can do the crash statistics.
    std::string QueryCrashID(const EXCEPTION_POINTERS* exptr)
    {
        wchar_t fault_module[MAX_PATH]{ 0 };
        auto fault_addr = exptr->ExceptionRecord->ExceptionAddress;
        if (!GetMappedFileNameW(GetCurrentProcess(), fault_addr, fault_module, MAX_PATH))
        {
            PLOG(WARNING) << "Failed to retrieve module path!";
            return lcpfw::kDummyCrashID;
        }

        auto module_name = base::FilePath(fault_module).BaseName();
        auto module_base_addr = reinterpret_cast<ULONG_PTR>(GetModuleHandleW(module_name.value().c_str()));
        auto module_offset = reinterpret_cast<ULONG_PTR>(fault_addr) - module_base_addr;

        return base::StringPrintf("%s+%x", module_name.AsUTF8Unsafe().c_str(), module_offset);
    }

    // "%localappdata%lcpfw/User Data/Crash Reports/crashinfo_pid", in the form:
    //      app version \n
    //      module_name+addr_offset
    void SaveExtraCrashInfo(const base::FilePath& dump_dir, EXCEPTION_POINTERS* exptr)
    {
        std::string crash_id = QueryCrashID(exptr);

        // 以崩溃进程PID为后缀创建崩溃模块、地址记录文件
        auto data_file = dump_dir.AppendASCII(GetCrashInfoFileName(::GetCurrentProcessId()));

        // Don't use file utils in base lib.
        // We might be in a thread that disallow I/O.
        std::ofstream out(data_file.value(), std::ios_base::out | std::ios_base::binary);
        out << AppContext::Current()->GetExecutableVersion() << "\n" << crash_id;
    }

    // "%localappdata%lcpfw/User Data/Crash Reports/crashlog_pid" >> logtext
    void SaveCurrentLogText(const base::FilePath& dump_dir)
    {
        base::FilePath log_file;
        base::PathService::Get(lcpfw::DIR_LOGS, &log_file);
        log_file = log_file.Append(lcpfw::kAppLogFileName);

        std::ifstream log_in(log_file.value(), std::ios_base::in | std::ios_base::ate);
        if (log_in)
        {
            auto eof_pos = log_in.tellg();
            int64_t startup_offset = lcpfw::GetStartupLogFileOffset();
            if (startup_offset > eof_pos)
            {
                startup_offset = 0;
            }
            int64_t buffer_size = eof_pos - startup_offset;
            if (buffer_size > lcpfw::kMaxCrashLogFileSize)
            {
                buffer_size = lcpfw::kMaxCrashLogFileSize;
                startup_offset = eof_pos - buffer_size;
            }
            log_in.seekg(0, log_in.beg);
            log_in.seekg(startup_offset);
            std::vector<char> buffer(buffer_size);
            log_in.read(&buffer[0], buffer_size);
            if (log_in)
            {
                // 以崩溃进程PID为后缀创建本次程序运行日志文件
                auto dmp_log_file = dump_dir.AppendASCII(GetCrashLogFileName(::GetCurrentProcessId()));
                std::ofstream log_out(dmp_log_file.value(), std::ios_base::out);
                if (log_out)
                {
                    log_out.write(&buffer[0], buffer.size());
                }
            }
        }
    }

    bool OnMinidumpGenerated(const wchar_t* dump_path, const wchar_t* minidump_id, void* context,
        EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
    {
        // "dump_path" name format like "guid.dmp"
        // We save extra crash information only when the dump file was successfully generated.
        if (succeeded)
        {
            SaveExtraCrashInfo(g_dump_dir, exinfo);
        }

        LOG(ERROR) << "Application crashed: on thread " << base::PlatformThread::GetName();

        // 将此次运行日志保存待上报
        logging::FlushLogFile();
        SaveCurrentLogText(g_dump_dir);

#if OS_WIN
        // No need to notify here, watcher process will do it.
        //::MessageBoxW(nullptr, L"Application crashed, you can restart application manually.", L"Application Crashed", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
#else
        //////////////////////////////////////////////////////////////////////////
#endif

        return succeeded;
    }

}   // namespace

CrashHandlerClient::CrashHandlerClient()
{
}

CrashHandlerClient::~CrashHandlerClient()
{
}

void CrashHandlerClient::Install()
{
#if defined(NDEBUG)
    _CrtSetReportMode(_CRT_ASSERT, 0);
#endif

    base::FilePath dump_dir_path;
    base::PathService::Get(lcpfw::DIR_CRASH_DUMPS, &dump_dir_path);
    g_dump_dir = dump_dir_path;

    auto dump_flags = DetermineDumpFlags();

    exception_handler_ = std::make_unique<google_breakpad::ExceptionHandler>(
        dump_dir_path.value(),
        nullptr,
        &OnMinidumpGenerated,
        nullptr,
        google_breakpad::ExceptionHandler::HANDLER_ALL,
        dump_flags,
        lcpfw::kExceptionHandlerPipeName,
        nullptr);
}

void CrashHandlerClient::UnInstall()
{
    exception_handler_ = nullptr;
}
