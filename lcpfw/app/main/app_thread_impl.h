#pragma once


#include "base/single_thread_task_runner.h"
#include "base/threading/thread.h"

#include "public/main/app_thread.h"


class AppThreadImpl
    : public AppThread
    , public base::Thread
{
public:
    explicit AppThreadImpl(AppThread::ID identifier);

    AppThreadImpl(AppThread::ID identifier,
        const std::string& name, scoped_refptr<base::SingleThreadTaskRunner> task_runner);
    virtual ~AppThreadImpl();

    static void ShutdownThreadPool();

    // cover
    scoped_refptr<base::SingleThreadTaskRunner> task_runner() const;
    base::PlatformThreadId GetThreadId() const;

    ID app_thread_id() const {
        return identifier_;
    }

protected:
    // base::Thread
    void Init() override;
    void Run(base::RunLoop* run_loop) override;
    void CleanUp() override;

private:
    friend class AppThread;

    void UIThreadRun(base::RunLoop* run_loop);
    void IOThreadRun(base::RunLoop* run_loop);
    /*void FileThreadRun(base::RunLoop* run_loop);
    void DBThreadRun(base::RunLoop* run_loop);*/

    static bool PostTaskHelper(
        AppThread::ID identifier,
        const base::Location &from_here,
        base::OnceClosure task,
        base::TimeDelta delay,
        bool nestable);

    void Initialize();

    friend class ContentTestSuiteBaseListener;
    static void FlushThreadPoolHelper();

    ID identifier_;
    base::PlatformThreadId thread_id_ = -1;
    scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
};
