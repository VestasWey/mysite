#pragma once

#include "base/command_line.h"
#include "base/system/system_monitor.h"
#include "base/task/sequence_manager/sequence_manager.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/timer/hi_res_timer_manager.h"

#include "content/main_function_params.h"


class AppMainParts;
class AppThread;
class AppSubThread;
class StartupTaskRunner;


class AppMainLoop
{
public:
    explicit AppMainLoop(const MainFunctionParams& parameters, 
        std::unique_ptr<base::ThreadPoolInstance::ScopedExecutionFence> fence);

    virtual ~AppMainLoop();

    static AppMainLoop *GetInstance();

    // Quick reference for initialization order:
    // Constructor
    // Init()
    // EarlyInitialization()
    // InitializeToolkit()
    // PreMainMessageLoopStart()
    // MainMessageLoopStart()
    //   InitializeMainThread()
    // PostMainMessageLoopStart()
    // CreateStartupTasks()
    //   PreCreateThreads()
    //   CreateThreads()
    //   PostCreateThreads()
    //   AppThreadsStarted()
    //     InitializeMojo()
    //   PreMainMessageLoopRun()
    // RunMainMessageLoopParts
    //   MainMessageLoopRun
    // PreShutdown
    // ShutdownThreadsAndCleanUp

    void Init();

    // Return value is exit status. Anything other than RESULT_CODE_NORMAL_EXIT
    // is considered an error.
    int EarlyInitialization();

    // Initializes the toolkit. Returns whether the toolkit initialization was
    // successful or not.
    bool InitializeToolkit();

    void PreMainMessageLoopStart();
    void MainMessageLoopStart();
    void PostMainMessageLoopStart();

    // Create and start running the tasks we need to complete startup. Note that
    // this can be called more than once (currently only on Android) if we get a
    // request for synchronous startup while the tasks created by asynchronous
    // startup are still running.
    void CreateStartupTasks();

    // Perform the default message loop run logic.
    void RunMainMessageLoopParts();

    void PreShutdown();

    // Performs the shutdown sequence, starting with PostMainMessageLoopRun
    // through stopping threads to PostDestroyThreads.
    void ShutdownThreadsAndCleanUp();

    int GetResultCode() const { return result_code_; }

    // Returns the task runner for tasks that that are critical to producing a new
    // CompositorFrame on resize. On Mac this will be the task runner provided by
    // WindowResizeHelperMac, on other platforms it will just be the thread task
    // runner.
    scoped_refptr<base::SingleThreadTaskRunner> GetResizeTaskRunner();

    AppMainParts* main_parts() { return parts_.get(); }

private:
    void InitializeMainThread();

    // Called just before creating the threads
    int PreCreateThreads();
    // Create all secondary threads.
    int CreateThreads();
    // Called just after creating the threads.
    int PostCreateThreads();
    // Called right after the browser threads have been started.
    int AppThreadsStarted();

    int PreMainMessageLoopRun();

    void MainMessageLoopRun();

private:
      // Members initialized on construction ---------------------------------------
    const MainFunctionParams& parameters_;
    const base::CommandLine& parsed_command_line_;
    int result_code_;
    bool created_threads_;  // True if the non-UI threads were created.
    // //content must be initialized single-threaded until
    // BrowserMainLoop::CreateThreads() as things initialized before it require an
    // initialize-once happens-before relationship with all eventual content tasks
    // running on other threads. This ScopedExecutionFence ensures that no tasks
    // posted to ThreadPool gets to run before CreateThreads(); satisfying this
    // requirement even though the ThreadPoolInstance is created and started
    // before content is entered.
    std::unique_ptr<base::ThreadPoolInstance::ScopedExecutionFence>
        scoped_execution_fence_;
        
    // BEST_EFFORT tasks are not allowed to run between //content initialization
    // and startup completion.
    //
    // TODO(fdoray): Move this to a more elaborate class that prevents BEST_EFFORT
    // tasks from running when resources are needed to respond to user actions.
    base::Optional<base::ThreadPoolInstance::ScopedBestEffortExecutionFence>
        scoped_best_effort_execution_fence_;

    // Unregister UI thread from hang watching on destruction.
    base::ScopedClosureRunner unregister_thread_closure_;

    // Members initialized in |PostMainMessageLoopStart()| -----------------------
    std::unique_ptr<base::SystemMonitor> system_monitor_;
    std::unique_ptr<base::HighResolutionTimerManager> hi_res_timer_manager_;
    
    std::unique_ptr<AppMainParts> parts_;
    std::unique_ptr<AppThread> main_thread_;
    std::vector<std::unique_ptr<AppThread>> worker_threads_;    // IO/FILE/DB... etc.

    // Members initialized in |CreateStartupTasks()| -----------------------------
    std::unique_ptr<StartupTaskRunner> startup_task_runner_;

    DISALLOW_COPY_AND_ASSIGN(AppMainLoop);
};
