#include "content/app_post_task_helper.h"

#include "base/lazy_instance.h"

namespace
{
    struct AppThreadGlobals
    {
        base::Lock lock;
        std::map<lcpfw::ThreadID, scoped_refptr<base::SingleThreadTaskRunner>> threads;
    };

    base::LazyInstance<AppThreadGlobals>::Leaky g_globals = LAZY_INSTANCE_INITIALIZER;
}


namespace lcpfw {

    // for special thread ===============================================================
    void RegisterAppThread(ThreadID id, scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    {
        if (!task_runner)
        {
            NOTREACHED() << "invalid argument";
            return;
        }

        AppThreadGlobals& globals = g_globals.Get();

        base::AutoLock lock(globals.lock);
        DCHECK(globals.threads.find(id) == globals.threads.end()) << "thread mustn't reg more than once";
        globals.threads[id] = task_runner;
    }

    scoped_refptr<base::SingleThreadTaskRunner> AppThreadTaskRunnerHandle(ThreadID id)
    {
        AppThreadGlobals& globals = g_globals.Get();

        base::AutoLock lock(globals.lock);
        DCHECK(globals.threads.find(id) != globals.threads.end()) << "thread not reg yet.";
        if (globals.threads.find(id) != globals.threads.end())
        {
            return globals.threads[id];
        }
        return nullptr;
    }

    void UnregisterAppThread(ThreadID id)
    {
        AppThreadGlobals& globals = g_globals.Get();

        base::AutoLock lock(globals.lock);
        auto iter = globals.threads.find(id);
        DCHECK(iter != globals.threads.end()) << "thread not reg yet.";
        if (iter != globals.threads.end())
        {
            globals.threads.erase(iter);
        }
    }

    bool BelongsToCurrentThread(ThreadID id)
    {
        auto task_runner = AppThreadTaskRunnerHandle(id);
        DCHECK(task_runner);
        if (task_runner)
        {
            return task_runner->BelongsToCurrentThread();
        }

        return false;
    }

    bool PostTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task)
    {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->PostTask(from_here, std::move(task));
    }

    bool PostDelayedTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay)
    {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->PostDelayedTask(from_here, std::move(task), delay);
    }

    bool PostTaskAndReply(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::OnceClosure reply)
    {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->PostTaskAndReply(from_here, std::move(task), std::move(reply));
    }

    bool PostNonNestableTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task)
    {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->PostNonNestableTask(from_here, std::move(task));
    }

    bool PostNonNestableDelayedTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay)
    {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->PostNonNestableDelayedTask(from_here, std::move(task), delay);
    }

    // just for current thread ===============================================================
    bool PostTask(const base::Location& from_here,
        base::OnceClosure task) {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->PostTask(from_here, std::move(task));
    }

    bool PostDelayedTask(const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay)
    {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->PostDelayedTask(from_here, std::move(task), delay);
    }

    bool PostTaskAndReply(const base::Location& from_here,
        base::OnceClosure task,
        base::OnceClosure reply)
    {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->PostTaskAndReply(from_here, std::move(task), std::move(reply));
    }

    bool PostNonNestableTask(const base::Location& from_here,
        base::OnceClosure task)
    {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->PostNonNestableTask(from_here, std::move(task));
    }

    bool PostNonNestableDelayedTask(const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay)
    {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->PostNonNestableDelayedTask(from_here, std::move(task), delay);
    }

}
