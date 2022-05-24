#include "content/app_message_loop.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/run_loop.h"
#include "base/system/sys_info.h"
#include "base/single_thread_task_runner.h"
#include "base/system/system_monitor.h"
#include "base/task/current_thread.h"
#include "base/task_runner.h"
#include "base/task/thread_pool/initialization_util.h"
#include "base/task/simple_task_executor.h"
#include "base/task/sequence_manager/sequence_manager.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/timer/hi_res_timer_manager.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
//#include "base/trace_event/memory_dump_manager.h"
//#include "base/trace_event/trace_event.h"

//#include "components/tracing/common/trace_startup_config.h"
//#include "components/tracing/common/tracing_switches.h"
//
//#include "net/socket/client_socket_factory.h"
//#include "net/ssl/ssl_config_service.h"
//#include "skia/ext/event_tracer_impl.h"
//#include "skia/ext/skia_memory_dump_provider.h"
//#include "sql/sql_memory_dump_provider.h"
//
//#include "ui/base/clipboard/clipboard.h"
//#include "ui/display/display_features.h"
//#include "ui/gfx/font_render_params.h"
//#include "ui/gfx/switches.h"
//
//#if defined(OS_MAC)
//#include "ui/accelerated_widget_mac/window_resize_helper_mac.h"
//#include "ui/base/l10n/l10n_util_mac.h"
//#endif
//
//#if defined(OS_WIN)
//#include <commctrl.h>
//#include <shellapi.h>
//#include <windows.h>
//
//#include "net/base/winsock_init.h"
//#include "ui/base/l10n/l10n_util_win.h"
//#endif
//
//#if defined(OS_WIN)
//#include "media/device_monitors/system_message_window_win.h"
//#elif defined(OS_MAC)
//#include "media/device_monitors/device_monitor_mac.h"
//#endif

#include "base/metrics/histogram_macros.h"

#include "common/app_context.h"
#include "common/app_result_codes.h"
#include "content/app_discardable_memory_allocator.h"
#include "content/app_thread.h"
#include "content/app_main_parts.h"
#include "content/app_post_task_helper.h"
#include "content/startup_task_runner.h"

namespace
{
    base::LazyInstance<base::AppDiscardableMemoryAllocator>::DestructorAtExit
        g_discardable_memory_allocator = LAZY_INSTANCE_INITIALIZER;

    // The currently-running BrowserMainLoop.  There can be one or zero.
    AppMainLoop* g_current_app_main_loop = nullptr;
}

// static
AppMainLoop* AppMainLoop::GetInstance()
{
    DCHECK(AppThread::CurrentlyOn(AppThread::UI));
    DCHECK(g_current_app_main_loop != nullptr);
    return g_current_app_main_loop;
}

AppMainLoop::AppMainLoop(const MainFunctionParams& parameters, 
    std::unique_ptr<base::ThreadPoolInstance::ScopedExecutionFence> scoped_execution_fence)
    : parameters_(parameters),
      parsed_command_line_(parameters.command_line),
      result_code_(lcpfw::ResultCodeNormalExit),
      created_threads_(false),
      scoped_execution_fence_(std::move(scoped_execution_fence)),
      scoped_best_effort_execution_fence_(base::in_place_t())
{
    DCHECK(!g_current_app_main_loop);
    DCHECK(scoped_execution_fence_) << "ThreadPool must be halted before kicking off content.";
    g_current_app_main_loop = this;

    // Register the UI thread for hang watching before it starts running and set
    // up a closure to automatically unregister when |this| is destroyed. This
    // works since the UI thread running the message loop is the main thread and
    // that makes it the same as the current one.
    if (base::HangWatcher::IsUIThreadHangWatchingEnabled()) {
        unregister_thread_closure_ = base::HangWatcher::RegisterThread(
            base::HangWatcher::ThreadType::kUIThread);
    }
}

AppMainLoop::~AppMainLoop()
{
    DCHECK_EQ(this, g_current_app_main_loop);

    //ui::Clipboard::DestroyClipboardForCurrentThread();

    g_current_app_main_loop = nullptr;
}

void AppMainLoop::Init()
{
    if (!parameters_.created_main_parts_closure.is_null())
    {
        parts_ = parameters_.created_main_parts_closure.Run(parameters_);
    }
    DCHECK(parts_);
}

// AppMainLoop stages ==================================================
//  Constructor
//  Init()
//  EarlyInitialization()
//  InitializeToolkit()
//  PreMainMessageLoopStart()
//  MainMessageLoopStart()
//    InitializeMainThread()
//  PostMainMessageLoopStart()
//  CreateStartupTasks()
//    PreCreateThreads()
//    CreateThreads()
//    PostCreateThreads()
//    AppThreadsStarted()
//      InitializeMojo()
//    PreMainMessageLoopRun()
//  RunMainMessageLoopParts
//    MainMessageLoopRun
//  PreShutdown
//  ShutdownThreadsAndCleanUp

int AppMainLoop::EarlyInitialization()
{
    if (parts_)
    {
        const int pre_early_init_error_code = parts_->PreEarlyInitialization();
        if (pre_early_init_error_code != lcpfw::ResultCodeNormalExit)
        {
            return pre_early_init_error_code;
        }
        
    }

  // Up the priority of the UI thread unless it was already high (since Mac
  // and recent versions of Android (O+) do this automatically).
#if !defined(OS_MAC)
    /*if (base::FeatureList::IsEnabled(features::kBrowserUseDisplayThreadPriority) &&
        base::PlatformThread::GetCurrentThreadPriority() < base::ThreadPriority::DISPLAY) {
        base::PlatformThread::SetCurrentThreadPriority(
            base::ThreadPriority::DISPLAY);
    }*/
#endif  // !defined(OS_MAC)

#if defined(OS_MAC) || defined(OS_LINUX) || defined(OS_CHROMEOS) || defined(OS_ANDROID)
    // We use quite a few file descriptors for our IPC as well as disk the disk
    // cache,and the default limit on the Mac is low (256), so bump it up.

    // Same for Linux. The default various per distro, but it is 1024 on Fedora.
    // Low soft limits combined with liberal use of file descriptors means power
    // users can easily hit this limit with many open tabs. Bump up the limit to
    // an arbitrarily high number. See https://crbug.com/539567
    base::IncreaseFdLimitTo(8192);
#endif

#if defined(OS_WIN)
    //net::EnsureWinsockInit();
#endif

    if (parts_)
    {
        parts_->PostEarlyInitialization();
    }

    return lcpfw::ResultCodeNormalExit;
}

bool AppMainLoop::InitializeToolkit()
{
    // TODO(evan): this function is rather subtle, due to the variety
    // of intersecting ifdefs we have.  To keep it easy to follow, there
    // are no #else branches on any #ifs.
    // TODO(stevenjb): Move platform specific code into platform specific Parts
    // (Need to add InitializeToolkit stage to BrowserParts).
    // See also GTK setup in EarlyInitialization, above, and associated comments.

  //#ifdef OS_WIN
  //    INITCOMMONCONTROLSEX config;
  //    config.dwSize = sizeof(config);
  //    config.dwICC = ICC_WIN95_CLASSES;
  //    if (!InitCommonControlsEx(&config))
  //    {
  //        PLOG(FATAL);
  //    }
  //#endif

    if (parts_)
    {
        parts_->ToolkitInitialized();
    }

    return true;
}

void AppMainLoop::PreMainMessageLoopStart() {
    //TRACE_EVENT0("startup", "AppMainLoop::MainMessageLoopStart:PreMainMessageLoopStart");
    if (parts_)
    {
        parts_->PreMainMessageLoopStart();
    }

//#ifdef OS_WIN
//    l10n_util::OverrideLocaleWithUILanguageList();
//#else
//    l10n_util::OverrideLocaleWithCocoaLocale();
//#endif
}

void AppMainLoop::MainMessageLoopStart()
{
    // DO NOT add more code here. Use PreMainMessageLoopStart() above or
    // PostMainMessageLoopStart() below.

    //TRACE_EVENT0("startup", "AppMainLoop::MainMessageLoopStart");
    DCHECK(base::CurrentUIThread::IsSet());
    InitializeMainThread();
}

void AppMainLoop::PostMainMessageLoopStart()
{
    {
        //TRACE_EVENT0("startup", "AppMainLoop::Subsystem:SystemMonitor");
        system_monitor_.reset(new base::SystemMonitor);
    }
    {
        //TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:PowerMonitor");
        if (!base::PowerMonitor::IsInitialized()) {
            base::PowerMonitor::Initialize(
                std::make_unique<base::PowerMonitorDeviceSource>());
        }
    }
    {
        //TRACE_EVENT0("startup", "AppMainLoop::Subsystem:HighResTimerManager");
        hi_res_timer_manager_.reset(new base::HighResolutionTimerManager);
    }

    // TODO(boliu): kSingleProcess check is a temporary workaround for
    // in-process Android WebView. crbug.com/503724 tracks proper fix.
    /*if (!parsed_command_line_.HasSwitch(switches::kSingleProcess)) {
        base::DiscardableMemoryAllocator::SetInstance(
            discardable_memory::DiscardableSharedMemoryManager::Get());
    }*/
    base::DiscardableMemoryAllocator::SetInstance(
        g_discardable_memory_allocator.Pointer());

    if (parts_)
    {
        parts_->PostMainMessageLoopStart();
    }

    // Enable memory-infra dump providers.
    // InitSkiaEventTracer();
    // base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
    //     skia::SkiaMemoryDumpProvider::GetInstance(), "Skia", nullptr);
    //base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
    //    sql::SqlMemoryDumpProvider::GetInstance(), "Sql", nullptr);
}

int AppMainLoop::PreCreateThreads()
{
    if (parts_)
    {
        result_code_ = parts_->PreCreateThreads();
    }

#if defined(OS_MAC)
    // The WindowResizeHelper allows the UI thread to wait on specific renderer
    // and GPU messages from the IO thread. Initializing it before the IO thread
    // starts ensures the affected IO thread messages always have somewhere to go.
    ui::WindowResizeHelperMac::Get()->Init(base::ThreadTaskRunnerHandle::Get());
#endif

    return result_code_;
}

void AppMainLoop::PreShutdown()
{
    // ui::Clipboard::OnPreShutdownForCurrentThread();
}

void AppMainLoop::CreateStartupTasks()
{
    DCHECK(!startup_task_runner_);
    startup_task_runner_ = std::make_unique<StartupTaskRunner>(
        base::OnceCallback<void(int)>(), base::ThreadTaskRunnerHandle::Get());

    StartupTask pre_create_threads = base::BindOnce(
        &AppMainLoop::PreCreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(std::move(pre_create_threads));

    StartupTask create_threads = base::BindOnce(
        &AppMainLoop::CreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(std::move(create_threads));

    StartupTask post_create_threads = base::BindOnce(
        &AppMainLoop::PostCreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(std::move(post_create_threads));

    StartupTask browser_thread_started = base::BindOnce(
        &AppMainLoop::AppThreadsStarted, base::Unretained(this));
    startup_task_runner_->AddTask(std::move(browser_thread_started));

    StartupTask pre_main_message_loop_run = base::BindOnce(
        &AppMainLoop::PreMainMessageLoopRun, base::Unretained(this));
    startup_task_runner_->AddTask(std::move(pre_main_message_loop_run));

    startup_task_runner_->RunAllTasksNow();
}

scoped_refptr<base::SingleThreadTaskRunner> AppMainLoop::GetResizeTaskRunner() {
#if defined(OS_MAC)
    scoped_refptr<base::SingleThreadTaskRunner> task_runner =
        ui::WindowResizeHelperMac::Get()->task_runner();
    // In tests, WindowResizeHelperMac task runner might not be initialized.
    return task_runner ? task_runner : base::ThreadTaskRunnerHandle::Get();
#else
    return base::ThreadTaskRunnerHandle::Get();
#endif
}

void testTimeout()
{
    auto beg = base::Time::Now();
    // 对各采样出现次数进行累计，输出当前作用域的执行耗时，毫秒为单位
    SCOPED_UMA_HISTOGRAM_TIMER("testTimeout().FunctionTime");
    Sleep(1000);
    SCOPED_UMA_HISTOGRAM_TIMER("testTimeout().FunctionTime");

    static int i = 0;
    printf("testTimeout: %d\n", ++i);

    // test
    //lcpfw::PostDelayedTask(FROM_HERE, base::BindOnce(testTimeout), base::TimeDelta::FromSeconds(1));

    // 对各采样出现次数进行累计，可得各采样值在总采样中的占比分布，可得所有采样值的平均值
    UMA_HISTOGRAM_BOOLEAN("Histogram.Bool", true);
    UMA_HISTOGRAM_BOOLEAN("Histogram.Bool", true);
    UMA_HISTOGRAM_BOOLEAN("Histogram.Bool", false);

    // 对各采样出现次数进行累计，可得各采样值在总采样中的占比分布。同样的label以第一次记录设定的最大值为阈值，后续采样指定新的最大值的调用不予记录。
    UMA_HISTOGRAM_EXACT_LINEAR("Histogram.Linear", 2, 10);  // 以第一次记录给定的10为该label的max阈值
    UMA_HISTOGRAM_EXACT_LINEAR("Histogram.Linear", 3, 20);  // 重新指定max阈值，该采样信息无效，不会被记录
    UMA_HISTOGRAM_EXACT_LINEAR("Histogram.Linear", 1, 10);
    UMA_HISTOGRAM_EXACT_LINEAR("Histogram.Linear", 19, 10);  // 采样值超过max阈值，按max阈值记录

    // 对各采样出现次数进行累计，可得各采样的在总采样中的总占比，采样范围定死为0~101，超过阈值的按阈值处理
    UMA_HISTOGRAM_PERCENTAGE("Histogram.Percent", 22);
    UMA_HISTOGRAM_PERCENTAGE("Histogram.Percent", 22);
    UMA_HISTOGRAM_PERCENTAGE("Histogram.Percent", 33);
    UMA_HISTOGRAM_PERCENTAGE("Histogram.Percent", 100);
    UMA_HISTOGRAM_PERCENTAGE("Histogram.Percent", 110);

    // 3KB=131072
    // 指定采样值，同时指定这个采样值发生的次数；限定采样值的最大阈值（超出阈值的采样值按阈值算）；指定一个除数，实际记录的采样数是传入的采样数和这个除数的商。
    UMA_HISTOGRAM_SCALED_EXACT_LINEAR("FooKiB", 1, 131072, 2, 1024);
    UMA_HISTOGRAM_SCALED_EXACT_LINEAR("FooKiB", 2, 131072, 2, 1024);
    UMA_HISTOGRAM_SCALED_EXACT_LINEAR("FooKiB", 33, 131072, 2, 1024);   // 实际采样值33超过设定的max阈值2，该采样信息将按采样值2进行记录

    // 对各采样出现次数进行累计，可得各采样的在总采样中的总占比，采样范围定死为100
    UMA_HISTOGRAM_COUNTS_100("My.Histogram", 22);
    UMA_HISTOGRAM_COUNTS_100("My.Histogram", 22);
    UMA_HISTOGRAM_COUNTS_100("My.Histogram", 33);

    // 对各采样出现次数进行累计，可得各采样值在总采样中的占比分布，可得所有采样值的平均值。输出的采样值单位为毫秒
    UMA_HISTOGRAM_TIMES("My.Timing.Histogram", base::TimeDelta::FromSeconds(1));
    UMA_HISTOGRAM_TIMES("My.Timing.Histogram", base::TimeDelta::FromMicroseconds(2000000));
    UMA_HISTOGRAM_TIMES("My.Timing.Histogram", base::TimeDelta::FromSeconds(3));
    UMA_HISTOGRAM_TIMES("My.Timing.Histogram", base::TimeDelta::FromSeconds(3));

    // 对各采样出现次数进行累计，可得各采样值在总采样中的占比分布，采样范围定死为1000~500000，可得所有采样值的平均值。输出的采样值单位为KB
    UMA_HISTOGRAM_MEMORY_KB("My.Memory.Histogram", 200);    // 小于min阈值的，按min阈值对待并记录
    UMA_HISTOGRAM_MEMORY_KB("My.Memory.Histogram", 1000);
    UMA_HISTOGRAM_MEMORY_KB("My.Memory.Histogram", 2200);
    UMA_HISTOGRAM_MEMORY_KB("My.Memory.Histogram", 2200);
    UMA_HISTOGRAM_MEMORY_KB("My.Memory.Histogram", 3300);


    auto del = base::Time::Now() - beg;
    LOG(INFO) << "testTimeout() coast: " << del.InMilliseconds();
}

int AppMainLoop::CreateThreads()
{
    // Release the ThreadPool's threads.
    scoped_execution_fence_.reset();

    // The |io_thread| can have optionally been injected into Init(), but if not,
    // create it here. Thre thread is only tagged as BrowserThread::IO here in
    // order to prevent any code from statically posting to it before
    // CreateThreads() (as such maintaining the invariant that PreCreateThreads()
    // et al. "happen-before" BrowserThread::IO is "brought up").
    //if (!io_thread_) {
    //  io_thread_ = BrowserTaskExecutor::CreateIOThread();
    //}
    //io_thread_->RegisterAsBrowserThread();
    //BrowserTaskExecutor::InitializeIOThread();

    base::Thread::Options default_options;
    base::Thread::Options io_message_loop_options;
    io_message_loop_options.message_pump_type = base::MessagePumpType::IO;
    base::Thread::Options ui_message_loop_options;
    ui_message_loop_options.message_pump_type = base::MessagePumpType::UI;

    for (size_t thread_id = AppThread::UI + 1;
         thread_id < AppThread::ID_COUNT;
         ++thread_id)
    {
        base::Thread::Options *options = &default_options;
        bool require_create = true;

        switch (thread_id)
        {
        case AppThread::IO:
            options = &io_message_loop_options;
            break;
        default:
            require_create = false;
            NOTREACHED();
            break;
        }

        if (require_create)
        {
            AppThread::ID id = static_cast<AppThread::ID>(thread_id);
            std::unique_ptr<AppThread> thread(new AppThread(id));
            thread->StartWithOptions(*options);
            worker_threads_.push_back(std::move(thread));
        }
    }

    lcpfw::PostTask(FROM_HERE, 
        base::BindOnce([](AppMainLoop* main_loop) {
            // Enable main thread and thread pool best effort queues. Non-best
            // effort queues will already have been enabled. This will enable
            // all queues on all browser threads, so we need to do this after
            // the threads have been created, i.e. here.
            //content::BrowserTaskExecutor::EnableAllQueues();
            main_loop->scoped_best_effort_execution_fence_.reset();
        },
        // Main thread tasks can't run after BrowserMainLoop destruction.
        // Accessing an Unretained pointer to BrowserMainLoop from a main
        // thread task is therefore safe.
            base::Unretained(this)));
    // test
    //lcpfw::PostTask(FROM_HERE, base::BindOnce(testTimeout));
    //lcpfw::PostDelayedTask(FROM_HERE, base::BindOnce(testTimeout), base::TimeDelta::FromSeconds(1500));
    base::PostTask(FROM_HERE, base::TaskTraits({ base::ThreadPool(), base::MayBlock() }), base::BindOnce(testTimeout));
    base::PostDelayedTask(FROM_HERE, base::TaskTraits({ base::ThreadPool(), base::MayBlock() }), base::BindOnce(testTimeout), base::TimeDelta::FromSeconds(1500));

    created_threads_ = true;
    return result_code_;
}

int AppMainLoop::PostCreateThreads() {
    if (parts_) {
        //TRACE_EVENT0("startup", "AppMainLoop::PostCreateThreads");
        parts_->PostCreateThreads();
    }

    return result_code_;
}

int AppMainLoop::PreMainMessageLoopRun()
{
    if (parts_)
    {
        parts_->PreMainMessageLoopRun();
    }

    // If the UI thread blocks, the whole UI is unresponsive. Do not allow
    // unresponsive tasks from the UI thread and instantiate a
    // responsiveness::Watcher to catch jank induced by any blocking tasks not
    // instrumented with ScopedBlockingCall's assert.
    /*base::DisallowUnresponsiveTasks();
    responsiveness_watcher_ = new content::responsiveness::Watcher();
    responsiveness_watcher_->SetUp();*/

    return result_code_;
}

void AppMainLoop::RunMainMessageLoopParts()
{
    // main thread must wait gpu resource
    base::ThreadRestrictions::SetWaitAllowed(true);

    bool ran_main_loop = false;
    if (parts_)
    {
        ran_main_loop = parts_->MainMessageLoopRun(&result_code_);
    }

    if (!ran_main_loop)
    {
        MainMessageLoopRun();
    }
}

void AppMainLoop::ShutdownThreadsAndCleanUp()
{
    if (!created_threads_)
    {
        // Called early, nothing to do
        return;
    }

    // Teardown may start in PostMainMessageLoopRun, and during teardown we
    // need to be able to perform IO.
    base::ThreadRestrictions::SetIOAllowed(true);
    /*lcpfw::PostTask(
        AppThread::IO, FROM_HERE,
        base::BindOnce(base::IgnoreResult(&base::ThreadRestrictions::SetIOAllowed), true));*/

    // Also allow waiting to join threads.
    // TODO(https://crbug.com/800808): Ideally this (and the above SetIOAllowed()
    // would be scoped allowances). That would be one of the first step to ensure
    // no persistent work is being done after ThreadPoolInstance::Shutdown() in
    // order to move towards atomic shutdown.
    base::ThreadRestrictions::SetWaitAllowed(true);
    /*lcpfw::PostTask(
        AppThread::IO, FROM_HERE,
        base::BindOnce(base::IgnoreResult(&base::ThreadRestrictions::SetWaitAllowed), true));*/

    if (parts_)
    {
        parts_->PostMainMessageLoopRun();
    }

    {
        base::ScopedAllowBaseSyncPrimitives allow_wait_sync;

        while (!worker_threads_.empty())
        {
            worker_threads_.pop_back();
        }
    }

    main_thread_.reset();

    if (parts_)
    {
        parts_->PostDestroyThreads();
    }
}

void AppMainLoop::InitializeMainThread()
{
    const char *kThreadName = "LcpfwMainThread";
    base::PlatformThread::SetName(kThreadName);

    // 主线程实例不用Start*，但需要为其指定ThreadTaskRunner
    // Register the main thread. The main thread's task runner should already have
    // been initialized in MainMessageLoopStart() (or before if
    // CurrentThread::Get() was externally provided).
    DCHECK(base::ThreadTaskRunnerHandle::IsSet());
    main_thread_.reset(new AppThread(AppThread::UI, kThreadName, base::ThreadTaskRunnerHandle::Get()));
}

int AppMainLoop::AppThreadsStarted()
{
    //content::HistogramSynchronizer::GetInstance();

//    // Alert the clipboard class to which threads are allowed to access the clipboard:
//    std::vector<base::PlatformThreadId> allowed_clipboard_threads;
//    // The current thread is the UI thread.
//    allowed_clipboard_threads.push_back(base::PlatformThread::CurrentId());
//#if defined(OS_WIN)
//    // On Windows, clipboard is also used on the IO thread.
//    for (auto& iter : worker_threads_)
//    {
//        if (iter.get())
//        {
//            allowed_clipboard_threads.push_back(iter->GetThreadId());
//        }
//    }
//#endif
//    ui::Clipboard::SetAllowedThreads(allowed_clipboard_threads);

#if defined(OS_MAC)
    // ThemeHelperMac::GetInstance();
#endif  // defined(OS_MAC)

    return result_code_;
}

void AppMainLoop::MainMessageLoopRun()
{
    base::RunLoop run_loop(base::RunLoop::Type::kNestableTasksAllowed);
    parts_->PreDefaultMainMessageLoopRun(run_loop.QuitClosure());

    lcpfw::PostDelayedTask(FROM_HERE, run_loop.QuitClosure(), base::TimeDelta::FromSeconds(5));

    run_loop.Run();
}
