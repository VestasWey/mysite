#include "common/app_logging.h"

#ifdef _MSC_VER
#include <Windows.h>
#include <shlobj.h>
#endif

#include <fstream>

#include "base/logging.h"
#include "base/path_service.h"
#include "base/files/file_util.h"
#include "base/metrics/statistics_recorder.h"
#include "base/threading/thread_restrictions.h"

#if defined(OS_WIN)
#include "base/logging_win.h"
#endif

#include "base/syslog_logging.h"
#include "base/base_switches.h"
#include "base/command_line.h"

#include "common/app_constants.h"
#include "common/app_paths.h"

namespace
{
    int64_t g_startup_log_file_offset = 0;

}   // namespace

namespace lcpfw
{
    base::FilePath GetAppLogDirectory()
    {
        base::FilePath fp;
        bool ret = base::PathService::Get(lcpfw::DIR_LOGS, &fp);
        DCHECK(ret);
        return fp;
    }

    void InitAppLogging(bool debug_mode)
    {
        logging::LoggingSettings logging_settings;

        if (debug_mode)
        {
            logging_settings.logging_dest = logging::LOG_TO_ALL;
        }

        auto log_dir = GetAppLogDirectory();
        auto log_file_path = log_dir.Append(lcpfw::kAppLogFileName);
        logging_settings.log_file_path = log_file_path.value().c_str();

        // 启用冗长日志记录，以便启用base::Histogram数据统计在日志里输出。
        // 也可以选择不在通用日志里输出，在程序合适的地方用base::StatisticsRecorder::ToJSON/WriteGraph()
        // 来获取数据后再自行输出到指定文件也可以。
#if _DEBUG
        if (!base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kV))
        {
            base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(switches::kV, "2");
        }
#endif

        logging::InitLogging(logging_settings);
        logging::SetLogItems(
            true,     // enable_process_id
            true,     // enable_thread_id
            true,     // enable_timestamp
            false);   // enable_tickcount

        {
            std::ifstream log_in(log_file_path.AsUTF8Unsafe().c_str(), std::ios_base::in | std::ios_base::ate);
            if (log_in)
            {
                g_startup_log_file_offset = log_in.tellg();
            }
        }

#if defined(OS_WIN)

        // {2F9A6165-18BB-4C02-A879-626C9AFE0E86}
        static const GUID kLcpfwTraceProviderName = {
            0x2f9a6165, 0x18bb, 0x4c02,
                { 0xa8, 0x79, 0x62, 0x6c, 0x9a, 0xfe, 0xe, 0x86 } };

        // Enable trace control and transport through event tracing for Windows.
        logging::LogEventProvider::Initialize(kLcpfwTraceProviderName);

        // Enable logging to the Windows Event Log.
//
// MessageId: BROWSER_CATEGORY
//
// MessageText:
//
// Browser Events
//
#define BROWSER_CATEGORY                 ((WORD)0x00000001L)

//
// MessageId: MSG_LOG_MESSAGE
//
// MessageText:
//
// %1!S!
//
#define MSG_LOG_MESSAGE                  ((DWORD)0x80000100L)

        logging::SetEventSource("lcpfw_app", BROWSER_CATEGORY, MSG_LOG_MESSAGE);
#endif

        base::StatisticsRecorder::InitLogOnShutdown();
    }

    void InitAppLogging()
    {
        // Always output into the output panel in visual studio when in debug.
#if !defined(NDEBUG)
        bool debug_mode = true;
#else
        bool debug_mode = false;
#endif

        InitAppLogging(debug_mode);
    }

    int64_t GetStartupLogFileOffset()
    {
        return g_startup_log_file_offset;
    }

    std::string GetCurrentLogText()
    {
        base::FilePath log_file = lcpfw::GetAppLogDirectory();
        log_file = log_file.Append(lcpfw::kAppLogFileName);

        std::ifstream log_in(log_file.AsUTF8Unsafe().c_str(), std::ios_base::in | std::ios_base::ate);
        if (log_in)
        {
            auto eof_pos = log_in.tellg();
            int64_t startup_offset = lcpfw::GetStartupLogFileOffset();
            if (startup_offset > eof_pos)
            {
                startup_offset = 0;
            }
            int64_t buffer_size = eof_pos - startup_offset;
            if (buffer_size > lcpfw::kMaxFileSizeAllowedToUpload)
            {
                buffer_size = lcpfw::kMaxFileSizeAllowedToUpload;
                startup_offset = eof_pos - buffer_size;
            }
            log_in.seekg(0, log_in.beg);
            log_in.seekg(startup_offset);
            std::string buffer(buffer_size, 0);
            log_in.read(&buffer[0], buffer_size);
            if (log_in)
            {
                return buffer;
            }
        }

        return {};
    }
}
