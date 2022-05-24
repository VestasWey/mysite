#include "content/app_thread.h"

#include <string>

#include "base/atomicops.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/task/current_thread.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread_restrictions.h"

#include "content/app_post_task_helper.h"

namespace
{
    static const char* g_app_thread_names[AppThread::ID_COUNT] =
    {
        "",  // UI main thread name assembled in app_message_loop.cc
        "LcpfwIOThread",  // IO
    };

    struct AppThreadGlobals
    {
        AppThreadGlobals()
        {
            memset(threads, 0, AppThread::ID_COUNT * sizeof(threads[0]));
        }

        base::Lock lock;
        AppThread* threads[AppThread::ID_COUNT];
    };

    base::LazyInstance<AppThreadGlobals>::Leaky g_globals = LAZY_INSTANCE_INITIALIZER;
}

AppThread::AppThread(ID identifier)
    : base::Thread(g_app_thread_names[identifier]),
    identifier_(identifier)
{
    Initialize();
}

AppThread::AppThread(ID identifier, const std::string& name, scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : base::Thread(name),
    identifier_(identifier),
    task_runner_(task_runner)
{
    Initialize();
}

AppThread::~AppThread()
{
    lcpfw::UnregisterAppThread(identifier_);

    AppThreadGlobals& globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    globals.threads[identifier_] = NULL;
#ifndef NDEBUG
    // Double check that the threads are ordered correctly in the enumeration.
    for (int i = identifier_ + 1; i < ID_COUNT; ++i)
    {
        DCHECK(!globals.threads[i]) << "Threads must be listed in the reverse order that they die";
    }
#endif
}

void AppThread::Initialize()
{
    {
        AppThreadGlobals& globals = g_globals.Get();

        base::AutoLock lock(globals.lock);
        DCHECK(identifier_ >= 0 && identifier_ < ID_COUNT);
        DCHECK(globals.threads[identifier_] == NULL);
        globals.threads[identifier_] = this;
    }

    if (identifier_ == UI)
    {
        DCHECK(task_runner_);
        lcpfw::RegisterAppThread(identifier_, task_runner());
    }
}

void AppThread::Init()
{
    if (AppThread::CurrentlyOn(AppThread::IO))
    {
        // Though this thread is called the "IO" thread, it actually just routes
        // messages around; it shouldn't be allowed to perform any blocking disk
        // I/O.
        base::ThreadRestrictions::SetIOAllowed(false);
        base::ThreadRestrictions::DisallowWaiting();
    }

    lcpfw::RegisterAppThread(identifier_, task_runner());
}

void AppThread::Run(base::RunLoop* run_loop)
{
    base::Thread::Run(run_loop);
}

void AppThread::CleanUp()
{
    lcpfw::UnregisterAppThread(identifier_);
}

scoped_refptr<base::SingleThreadTaskRunner> AppThread::task_runner() const
{
    if (identifier_ != AppThread::UI)
    {
        return base::Thread::task_runner();
    }

    DCHECK(task_runner_);
    return task_runner_;
}

base::PlatformThreadId AppThread::GetThreadId() const
{
    if (identifier_ != AppThread::UI)
    {
        return base::Thread::GetThreadId();
    }

    return base::PlatformThread::CurrentId();
}

// static
bool AppThread::PostTaskHelper(
    AppThread::ID identifier,
    const base::Location& from_here,
    base::OnceClosure task,
    base::TimeDelta delay,
    bool nestable)
{
    DCHECK(identifier >= 0 && identifier < ID_COUNT);

    AppThread::ID current_thread;
    bool target_thread_outlives_current =
        GetCurrentThreadIdentifier(&current_thread) &&
        current_thread >= identifier;

    AppThreadGlobals& globals = g_globals.Get();
    if (!target_thread_outlives_current)
    {
        globals.lock.Acquire();
    }

    scoped_refptr<base::SingleThreadTaskRunner> task_runner =
        globals.threads[identifier] ? globals.threads[identifier]->task_runner() : nullptr;
    if (task_runner)
    {
        if (nestable)
        {
            task_runner->PostDelayedTask(from_here, std::move(task), delay);
        }
        else
        {
            task_runner->PostNonNestableDelayedTask(from_here, std::move(task), delay);
        }
    }

    if (!target_thread_outlives_current)
    {
        globals.lock.Release();
    }

    return !!task_runner;
}


// static for ThreadPool
bool AppThread::PostBlockingPoolTask(
    const base::Location& from_here,
    base::OnceClosure task)
{
    if (base::ThreadPoolInstance::Get())
    {
        return base::ThreadPool::PostTask(from_here, { base::ThreadPool(), base::MayBlock() }, std::move(task));
    }
    else
    {
        NOTREACHED();
    }
    return false;
}

bool AppThread::PostBlockingPoolTaskAndReply(
    const base::Location& from_here,
    base::OnceClosure task,
    base::OnceClosure reply)
{
    if (base::ThreadPoolInstance::Get())
    {
        return base::ThreadPool::PostTaskAndReply(from_here, { base::ThreadPool(), base::MayBlock() }, std::move(task), std::move(reply));
    }
    else
    {
        NOTREACHED();
    }
    return false;
}

// static
bool AppThread::IsThreadInitialized(ID identifier)
{
    if (!g_globals.IsCreated())
    {
        return false;
    }

    AppThreadGlobals& globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] != NULL;
}

// static
bool AppThread::CurrentlyOn(ID identifier)
{
    // We shouldn't use MessageLoop::current() since it uses LazyInstance which
    // may be deleted by ~AtExitManager when a WorkerPool thread calls this
    // function.
    // http://crbug.com/63678
    //base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
    AppThreadGlobals& globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] &&
        globals.threads[identifier]->GetThreadId() == base::PlatformThread::CurrentId();
}

// static
bool AppThread::IsMessageLoopValid(ID identifier)
{
    if (!g_globals.IsCreated())
    {
        return false;
    }

    AppThreadGlobals& globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] &&
        globals.threads[identifier]->task_runner();
}

// static
bool AppThread::GetCurrentThreadIdentifier(ID* identifier)
{
    if (!g_globals.IsCreated())
    {
        return false;
    }

    AppThreadGlobals& globals = g_globals.Get();
    for (int i = 0; i < ID_COUNT; ++i)
    {
        if (globals.threads[i] &&
            globals.threads[i]->GetThreadId() == base::PlatformThread::CurrentId())
        {
            *identifier = globals.threads[i]->identifier_;
            return true;
        }
    }

    return false;
}

// static
scoped_refptr<base::SingleThreadTaskRunner>
AppThread::GetThreadTaskRunner(ID identifier)
{
    AppThreadGlobals& globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier]->task_runner();
}

// static
//void AppThread::FlushThreadPoolHelper()
//{
//    if (!g_globals.IsCreated())
//    {
//        return;
//    }
//    if (base::ThreadPoolInstance::Get())
//    {
//        base::ThreadPoolInstance::Get()->FlushForTesting();
//    }
//}
