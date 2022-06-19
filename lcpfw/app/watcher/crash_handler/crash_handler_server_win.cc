#include "crash_handler_server.h"

#include <atomic>
#include <string>

#include <Windows.h>

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/files/file_path.h"

#include "client/windows/crash_generation/client_info.h"
//#include "client/windows/crash_generation/crash_generation_server.h"

#include "common/app_constants.h"
#include "content/app_thread.h"
#include "content/app_post_task_helper.h"
#include "common/app_paths.h"
#include "common/app_crash_helper.h"
#include "base/files/file_util.h"

#include "watcher/ui/crash/crash_notify_window.h"

namespace {

    const int64_t kPendingInterval = 5;   // in seconds.

    std::atomic<bool> g_client_bound{ false };
    std::wstring g_dump_file_path;
    base::RepeatingClosure g_quit_closure;

    void QuitNow()
    {
        if (!g_quit_closure.is_null())
        {
            g_quit_closure.Run();
        }
    }

    void QuitWhenNotBound()
    {
        if (!g_client_bound.load(std::memory_order::memory_order_acquire))
        {
            LOG(WARNING) << "No main process is bound; Quit now.";
            QuitNow();
        }
    }

    // "%localappdata%lcpfw/User Data/Crash Reports/guid" >> exe-version \n module+offset
    void SetupCrashInfoFile(const base::FilePath& dump_file, DWORD client_pid)
    {
        auto data_path = dump_file.DirName().AppendASCII(GetCrashInfoFileName(client_pid));
        base::Move(data_path, dump_file.RemoveExtension());
    }

    // "%localappdata%lcpfw/User Data/Crash Reports/guid.log" >> logtext
    void SetupCrashLogFile(const base::FilePath& dump_file, DWORD client_pid)
    {
        auto data_path = dump_file.DirName().AppendASCII(GetCrashLogFileName(client_pid));
        base::Move(data_path, dump_file.RemoveExtension().AddExtension(L"log"));
    }

    void ShowCrashNotifyWindow()
    {
        CrashNotifWindow::ShowWindow(g_quit_closure);
    }

}   // namespace


CrashHandlerServer::CrashHandlerServer(base::RepeatingClosure quit_closure)
{
    DCHECK(!quit_closure.is_null());
    g_quit_closure = quit_closure;
    base::FilePath dump_dir_path;
    base::PathService::Get(lcpfw::DIR_CRASH_DUMPS, &dump_dir_path);

    server_ = std::make_unique<google_breakpad::CrashGenerationServer>(
        lcpfw::kExceptionHandlerPipeName,
        nullptr,
        &CrashHandlerServer::OnClientConnected,
        nullptr,
        &CrashHandlerServer::OnClientCrashed,
        nullptr,
        &CrashHandlerServer::OnClientExited,
        nullptr,
        nullptr,
        nullptr,
        true,
        &dump_dir_path.value());
}

bool CrashHandlerServer::Start()
{
    bool started = server_->Start();
    if (started)
    {
        lcpfw::PostDelayedTask(AppThread::UI,
            FROM_HERE,
            base::Bind(QuitWhenNotBound),
            base::TimeDelta::FromSeconds(kPendingInterval));
    }

    return started;
}

void CrashHandlerServer::Stop()
{
    server_ = nullptr;
}

// static
void CrashHandlerServer::OnClientConnected(void* context, const google_breakpad::ClientInfo* client_info)
{
    LOG(INFO) << "main process connected(pid:" << client_info->pid() << ")";
    g_client_bound.store(true, std::memory_order::memory_order_release);
}

// static
void CrashHandlerServer::OnClientCrashed(void* context, const google_breakpad::ClientInfo* client_info,
    const std::wstring* dump_path)
{
    LOG(INFO) << "main process crashed(pid:" << client_info->pid() << ")";
    g_dump_file_path = *dump_path;


    // Show notify msgbox, allow feedback and choose restart app
    lcpfw::PostTask(AppThread::UI, FROM_HERE, base::Bind(ShowCrashNotifyWindow));
}

// static
void CrashHandlerServer::OnClientExited(void* context, const google_breakpad::ClientInfo* client_info)
{
    if (!g_dump_file_path.empty())
    {
        SetupCrashInfoFile(base::FilePath(g_dump_file_path), client_info->pid());
        SetupCrashLogFile(base::FilePath(g_dump_file_path), client_info->pid());
    }

    LOG(INFO) << "main process disconnected(pid:" << client_info->pid() << "), watcher quit too.";
    lcpfw::PostTask(AppThread::UI, FROM_HERE, base::Bind(QuitNow));
}
