#pragma once

#include <string>

#include "base/callback.h"
#include "base/location.h"
#include "base/task_runner_util.h"
#include "base/task/post_task.h"
#include "base/time/time.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"


class AppThread : public base::Thread
{
public:
    enum ID
    {
        UI,
        IO,
        ID_COUNT
    };

    // cover base::Thread
    scoped_refptr<base::SingleThreadTaskRunner> task_runner() const;
    base::PlatformThreadId GetThreadId() const;

    ID app_thread_id() const {
        return identifier_;
    }

    static bool PostBlockingPoolTask(const base::Location &from_here,
                                     base::OnceClosure task);
    static bool PostBlockingPoolTaskAndReply(
        const base::Location &from_here,
        base::OnceClosure task,
        base::OnceClosure reply);

    static bool IsThreadInitialized(ID identifier);

    static bool CurrentlyOn(ID identifier);

    static bool IsMessageLoopValid(ID identifier);

    static bool GetCurrentThreadIdentifier(ID *identifier);

    static scoped_refptr<base::SingleThreadTaskRunner> GetThreadTaskRunner(ID identifier);

    template<ID thread>
    struct DeleteOnThread
    {
        template<typename T>
        static void Destruct(const T *x)
        {
            if (CurrentlyOn(thread))
            {
                delete x;
            }
            else
            {
                if (!DeleteSoon(thread, FROM_HERE, x))
                {
                    LOG(ERROR) << "DeleteSoon failed on thread " << thread;
                }
            }
        }
    };

    struct DeleteOnUIThread : public DeleteOnThread<UI> { };
    struct DeleteOnIOThread : public DeleteOnThread<IO> { };

protected:
    static bool PostTaskHelper(
        AppThread::ID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay,
        bool nestable);

    AppThread(ID identifier);
    // use for init main thread only
    AppThread(ID identifier, const std::string& name, scoped_refptr<base::SingleThreadTaskRunner> task_runner);
    ~AppThread() override;

    void Initialize();

    // base::Thread
    void Init() override;
    void Run(base::RunLoop* run_loop) override;
    void CleanUp() override;

private:
    friend class AppMainLoop;
    friend struct std::default_delete<AppThread>;

    ID identifier_;
    scoped_refptr<base::SingleThreadTaskRunner> task_runner_;   // ref to base::ThreadTaskRunnerHandle::Get()

    DISALLOW_COPY_AND_ASSIGN(AppThread);
};

