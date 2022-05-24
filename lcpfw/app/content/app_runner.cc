#include "content/app_runner.h"

#ifdef OS_WIN
#include <atlbase.h>
#include "base/logging_win.h"
#endif

#include "base/command_line.h"
#include "base/process/memory.h"
#include "base/task/thread_pool/thread_pool_instance.h"
//#include "content/browser/notification_service_impl.h"

//#include "app_task_environment.h"
#include "common/app_features.h"
#include "common/app_paths.h"
#include "content/app_message_loop.h"
#include "content/app_task_environment.h"


namespace {

base::LazyInstance<base::AtomicFlag>::Leaky g_exited_main_message_loop;

}

class AppMainRunnerImpl : public AppMainRunner 
{
public:
    AppMainRunnerImpl()
        : task_environment_(std::make_unique<base::AppTaskEnvironment>(
            base::AppTaskEnvironment::MainThreadType::UI,   // 主线程消息循环方式，Windows下为MessageWindow方式
            base::AppTaskEnvironment::ThreadingMode::MULTIPLE_THREADS)),    // 开线程池
          scoped_execution_fence_(std::make_unique<base::ThreadPoolInstance::ScopedExecutionFence>())
    {
#ifdef OS_WIN
        GUID guid;
        com_module_.Init(NULL, ::GetModuleHandle(NULL), &guid);
#endif
    }

    virtual ~AppMainRunnerImpl()
    {
        if (initialization_started_ && !is_shutdown_)
        {
            Shutdown();
        }
    }

    int Initialize(const MainFunctionParams& params)
    {
        initialization_started_ = true;

        //notification_service_.reset(new NotificationServiceImpl);

#if defined(OS_WIN)
        // Ole must be initialized before starting message pump, so that TSF
        // (Text Services Framework) module can interact with the message pump
        // on Windows 8 Metro mode.
        //ole_initializer_.reset(new ui::ScopedOleInitializer);
#endif  // OS_WIN

        AppFeatures::Init();

        main_loop_.reset(new AppMainLoop(params, std::move(scoped_execution_fence_)));
        main_loop_->Init();
        
        const int early_init_error_code = main_loop_->EarlyInitialization();
        if (early_init_error_code > 0)
            return early_init_error_code;

        // Must happen before we try to use a message loop or display any UI.
        if (!main_loop_->InitializeToolkit())
            return 1;

        main_loop_->PreMainMessageLoopStart();
        main_loop_->MainMessageLoopStart();
        main_loop_->PostMainMessageLoopStart();

        main_loop_->CreateStartupTasks();
        int result_code = main_loop_->GetResultCode();
        if (result_code > 0)
            return result_code;
        
        // Return -1 to indicate no early termination.
        return -1;
    }

    int Run()
    {
        DCHECK(initialization_started_);
        DCHECK(!is_shutdown_);
        main_loop_->RunMainMessageLoopParts();

        // 到这里主线程的RunLoop已经退出，把任务队列里还没来得及执行的任务信息打出来
        task_environment_->DescribeCurrentTasks();

        return main_loop_->GetResultCode();
    }

    void Shutdown()
    {
        DCHECK(initialization_started_);
        DCHECK(!is_shutdown_);

        main_loop_->PreShutdown();

        // The trace event has to stay between profiler creation and destruction.
        //TRACE_EVENT0("shutdown", "AppMainRunner");
        g_exited_main_message_loop.Get().Set();
        
        main_loop_->ShutdownThreadsAndCleanUp();

#if defined(OS_WIN)
        //ole_initializer_.reset(NULL);
#endif

        main_loop_.reset();

        task_environment_.reset();

        //notification_service_.reset(nullptr);

        is_shutdown_ = true;
    }

private:
    bool initialization_started_ = false;
    bool is_shutdown_ = false;

    std::unique_ptr<base::AppTaskEnvironment> task_environment_;

    // Prevents execution of ThreadPool tasks from the moment content is
    // entered. Handed off to |main_loop_| later so it can decide when to release
    // worker threads again.
    std::unique_ptr<base::ThreadPoolInstance::ScopedExecutionFence> scoped_execution_fence_;

    //std::unique_ptr<content::NotificationServiceImpl> notification_service_;
    std::unique_ptr<AppMainLoop> main_loop_;

#ifdef OS_WIN
    //std::unique_ptr<ui::ScopedOleInitializer> ole_initializer_;
    CComModule com_module_;
#endif

    DISALLOW_COPY_AND_ASSIGN(AppMainRunnerImpl);
};

// static
std::unique_ptr<AppMainRunner> AppMainRunner::Create()
{
    return std::make_unique<AppMainRunnerImpl>();
}

// static
bool AppMainRunner::ExitedMainMessageLoop() {
  return g_exited_main_message_loop.IsCreated() &&
         g_exited_main_message_loop.Get().IsSet();
}
