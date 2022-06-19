#include "watcher/app_watcher_parts_impl.h"

#include <future>

#ifdef OS_WIN
//#include "base/win/scoped_handle.h"
#include "base/win/windows_version.h"
#endif

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/system/sys_info.h"
#include "base/task/current_thread.h"

#include "common/app_context.h"
#include "common/app_paths.h"
#include "common/app_pref_names.h"
#include "common/app_result_codes.h"
//#include "content/public/notification/notification_service.h"
#include "common/app_constants.h"
#include "content/app_post_task_helper.h"
#include "content/app_thread.h"
#include "watcher/app_watcher_extra_parts_views.h"
#include "watcher/crash_handler/crash_handler_server.h"



namespace
{
    std::unique_ptr<base::RunLoop> g_run_loop;

#if OS_WIN
    std::unique_ptr<base::WaitableEvent> g_crash_ready_evt;
#endif

    base::WaitableEvent g_shutdown_evt;
    std::future<void> g_wait_pps_task;  // wait parent process signal task
}

std::unique_ptr<AppMainParts> CreateAppMainParts(const MainFunctionParams& main_function_params)
{
    std::unique_ptr<WatcherMainPartsImpl> main_parts = std::make_unique<WatcherMainPartsImpl>(main_function_params);

    // Add some ExtraParts
    main_parts->AddParts(std::make_unique<WatcherMainExtraPartsViews>());

    return main_parts;
}

WatcherMainPartsImpl::WatcherMainPartsImpl(const MainFunctionParams& main_function_params)
    : parameters_(main_function_params),
    parsed_command_line_(main_function_params.command_line),
    result_code_(lcpfw::ResultCodeNormalExit)
{
}

WatcherMainPartsImpl::~WatcherMainPartsImpl()
{
    // Delete parts in the reverse of the order they were added.
    while (!app_extra_parts_.empty())
    {
        app_extra_parts_.pop_back();
    }
}

// AppMainParts implementation ------------------------------------

int WatcherMainPartsImpl::PreEarlyInitialization()
{
#if OS_WIN
    HANDLE evt = ::CreateEventW(nullptr, true, false, lcpfw::kExceptionHandlerReadyEventName);
    if (evt == nullptr)
    {
        LOG(WARNING) << "Create ExceptionHandlerReadyEvent failed.";
    }
    g_crash_ready_evt = std::make_unique<base::WaitableEvent>(base::win::ScopedHandle(evt));
#endif

    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreEarlyInitialization();
    }

    base::PathService::Get(lcpfw::DIR_USER_DATA, &user_data_dir_);
    DCHECK(!user_data_dir_.empty());

    OnLocalStateLoaded();

    return lcpfw::ResultCodeNormalExit;
}

void WatcherMainPartsImpl::PostEarlyInitialization()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostEarlyInitialization();
    }
}

void WatcherMainPartsImpl::ToolkitInitialized()
{
    //ntf_service_.reset(content::NotificationService::Create());

    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->ToolkitInitialized();
    }
}

void WatcherMainPartsImpl::PreMainMessageLoopStart()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreMainMessageLoopStart();
    }
}

void WatcherMainPartsImpl::PostMainMessageLoopStart()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostMainMessageLoopStart();
    }
}

int WatcherMainPartsImpl::PreCreateThreads()
{
    result_code_ = PreCreateThreadsImpl();

    if (result_code_ == lcpfw::ResultCodeNormalExit)
    {
        for (size_t i = 0; i < app_extra_parts_.size(); ++i)
        {
            app_extra_parts_[i]->PreCreateThreads();
        }
    }

    return result_code_;
}

int WatcherMainPartsImpl::PreCreateThreadsImpl()
{
    run_message_loop_ = false;

    // Create the RunLoop for MainMessageLoopRun() to use, and pass a copy of
    // its QuitClosure to the BrowserProcessImpl to call when it is time to exit.
    DCHECK(!g_run_loop);
    g_run_loop.reset(new base::RunLoop(base::RunLoop::Type::kNestableTasksAllowed));

#if defined(OS_MAC)
    // Get the Keychain API to register for distributed notifications on the main
    // thread, which has a proper CFRunloop, instead of later on the I/O thread,
    // which doesn't. This ensures those notifications will get delivered
    // properly. See issue 37766.
    // (Note that the callback mask here is empty. I don't want to register for
    // any callbacks, I just want to initialize the mechanism.)
    SecKeychainAddCallback(&KeychainCallback, 0, nullptr);
#endif  // defined(OS_MAC)

    return lcpfw::ResultCodeNormalExit;
}

void WatcherMainPartsImpl::PostCreateThreads()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostCreateThreads();
    }
}

void WatcherMainPartsImpl::PreMainMessageLoopRun()
{
    result_code_ = PreMainMessageLoopRunImpl();

    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreMainMessageLoopRun();
    }
}

// PreMainMessageLoopRun calls these extra stages in the following order:
//  PreMainMessageLoopRunImpl()
//   ... initial setup, including browser_process_ setup.

int WatcherMainPartsImpl::PreMainMessageLoopRunImpl()
{
    // 
    auto pid_str = parsed_command_line_.GetSwitchValueASCII(lcpfw::kSwitchParentPID);
    uint64_t pid = 0;
    if (pid_str.empty() || !base::StringToUint64(pid_str, &pid))
    {
        return lcpfw::ResultCodeErrorOccurred;
    }
    else
    {
        g_wait_pps_task = std::async(std::launch::async, [](uint64_t pid, base::RepeatingClosure quit_closure) {
#if OS_WIN
            HANDLE process_handle = ::OpenProcess(SYNCHRONIZE, false, pid);
            if (process_handle)
            {
                HANDLE handles[2] = { process_handle , g_shutdown_evt.handle() };
                DWORD dwRet = WaitForMultipleObjects(2, handles, false, INFINITE);
                switch (dwRet)
                {
                case WAIT_OBJECT_0:
                    // Main process exit (process_handle signal)
                    LOG(INFO) << "main process exit, watcher shutdown right now";
                    break;
                case WAIT_OBJECT_0 + 1:
                    // Current process will shutdown (g_shutdown_evt signal)
                    // e.g. main process is crashed but not timely terminate for some unexpected reason.
                    LOG(INFO) << "main process crashed, may handled by CrashServer, watcher should shutdown right now.";
                    break;
                default:
                    LOG(WARNING) << "wait for main process exit or CrashServer shutdown failed.";
                    break;
                }

                ::CloseHandle(process_handle);
            }
#elif OS_MAC
            HANDLE process_handle = ::OpenProcess(SYNCHRONIZE, false, browser_process_id);
            if (process_handle)
            {
                HANDLE handles[2] = { process_handle , hShutdownEvent };
                DWORD dwRet = WaitForMultipleObjects(2, handles, false, INFINITE);
                if (dwRet == WAIT_OBJECT_0)
                {
                    // 父进程退了，向我们自己的主线程（CEF主线程）发退出消息
                    PostThreadMessage(main_thread_id, WM_QUIT, 0, 0);
                    // 停顿个几秒钟，让CEF按自己的方式退出，不然就直接强退了（虽然CEF自己也是强退）
                    dwRet = WaitForSingleObject(hShutdownEvent, 5000);
                    if (dwRet != WAIT_OBJECT_0)
                    {
                        TerminateProcess(GetCurrentProcess(), (UINT)-1);
                    }
                }

                ::CloseHandle(process_handle);
            }
#endif
            // No matter how WaitXXX return, just signal shutdown and quit runloop.
            g_shutdown_evt.Signal();

            LOG(INFO) << "main process maybe quit, watcher quit too.";
            lcpfw::PostTask(AppThread::UI, FROM_HERE, quit_closure);
            },
            pid, base::Bind(&WatcherMainPartsImpl::OnParentProcessExit, base::Unretained(this)));
    }

#if defined(OS_WIN)
    //ui::SelectFileDialog::SetFactory(new AppSelectFileDialogFactory());
#endif  // defined(OS_WIN)

    crash_handler_server_ = std::make_unique<CrashHandlerServer>(
        base::Bind(&WatcherMainPartsImpl::OnParentProcessExit, base::Unretained(this)));
    crash_handler_server_->Start();
    run_message_loop_ = true;

    // Set google_breakpad::CrashGenerationServer ready flag, so that main process can relieve from WaitForCrashServerReady timely.
    g_crash_ready_evt->Signal();

    return lcpfw::ResultCodeNormalExit;
}

bool WatcherMainPartsImpl::MainMessageLoopRun(int* result_code)
{
    *result_code = result_code_;
    if (!run_message_loop_)
    {
        return false;  // Run the AppMessageLoop default message loop.
    }

    if (g_shutdown_evt.IsSignaled())
    {
        return true;
    }

    if (g_run_loop)
    {
        DCHECK(base::CurrentUIThread::IsSet());
        g_run_loop->Run();
    }

    return true;
}

void WatcherMainPartsImpl::PostMainMessageLoopRun()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i) {
        app_extra_parts_[i]->PostMainMessageLoopRun();
    }
}

void WatcherMainPartsImpl::PostDestroyThreads()
{
    g_run_loop.reset();

    //device_event_log::Shutdown();
}

void WatcherMainPartsImpl::OnLocalStateLoaded()
{
}

void WatcherMainPartsImpl::AddParts(std::unique_ptr<AppMainExtraParts> parts)
{
    app_extra_parts_.push_back(std::move(parts));
}

void WatcherMainPartsImpl::OnParentProcessExit()
{
    // Signal and wait "g_wait_pps_task" exit.
    g_shutdown_evt.Signal();
    g_wait_pps_task.wait();

    if (crash_handler_server_)
    {
        crash_handler_server_->Stop();
        crash_handler_server_ = nullptr;
    }

    // Quit message loop
    g_run_loop->QuitWhenIdle();
}
