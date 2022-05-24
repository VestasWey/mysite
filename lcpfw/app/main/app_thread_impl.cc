#include "main/app_thread_impl.h"

#include <string>

#include "base/atomicops.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/task/current_thread.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread_restrictions.h"

#include "public/main/app_thread_delegate.h"

namespace
{
    static const char *g_app_thread_names[AppThread::ID_COUNT] =
    {
        "",  // UI (name assembled in app_main.cc).
        "IOThread",  // IO
        //"DBThread",  // DB
        //"FileThread",  // FILE
    };

    struct AppThreadGlobals
    {
        AppThreadGlobals()
        {
            memset(threads, 0, AppThread::ID_COUNT * sizeof(threads[0]));
            memset(thread_delegates, 0,
                   AppThread::ID_COUNT * sizeof(thread_delegates[0]));
        }

        base::Lock lock;

        AppThreadImpl *threads[AppThread::ID_COUNT];

        AppThreadDelegate *thread_delegates[AppThread::ID_COUNT];
    };

    base::LazyInstance<AppThreadGlobals>::Leaky g_globals = LAZY_INSTANCE_INITIALIZER;

}

AppThreadImpl::AppThreadImpl(ID identifier)
    : base::Thread(g_app_thread_names[identifier])
    , identifier_(identifier)
{
    Initialize();
}

AppThreadImpl::AppThreadImpl(ID identifier,
    const std::string& name, scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : base::Thread(name), 
    identifier_(identifier),
    task_runner_(task_runner)
{
    Initialize();
    thread_id_ = base::PlatformThread::CurrentId();
}

// static
void AppThreadImpl::ShutdownThreadPool()
{
    if (base::ThreadPoolInstance::Get())
    {
        base::ThreadPoolInstance::Get()->Shutdown();
    }
}

// static
void AppThreadImpl::FlushThreadPoolHelper()
{
    if (!g_globals.IsCreated())
    {
        return;
    }
    if (base::ThreadPoolInstance::Get())
    {
        base::ThreadPoolInstance::Get()->FlushForTesting();
    }
}

NOINLINE void AppThreadImpl::UIThreadRun(base::RunLoop* run_loop)
{
    volatile int line_number = __LINE__;
    base::Thread::Run(run_loop);
    CHECK_GT(line_number, 0);
}

NOINLINE void AppThreadImpl::IOThreadRun(base::RunLoop* run_loop)
{
    volatile int line_number = __LINE__;
    base::Thread::Run(run_loop);
    CHECK_GT(line_number, 0);
}

//NOINLINE void AppThreadImpl::DBThreadRun(base::RunLoop* run_loop)
//{
//    volatile int line_number = __LINE__;
//    Thread::Run(run_loop);
//    CHECK_GT(line_number, 0);
//}
//
//NOINLINE void AppThreadImpl::FileThreadRun(base::RunLoop* run_loop)
//{
//    volatile int line_number = __LINE__;
//    Thread::Run(run_loop);
//    CHECK_GT(line_number, 0);
//}

void AppThreadImpl::Init()
{
    AppThreadGlobals &globals = g_globals.Get();

    using base::subtle::AtomicWord;
    AtomicWord *storage =
        reinterpret_cast<AtomicWord *>(&globals.thread_delegates[identifier_]);
    AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
    AppThreadDelegate *delegate =
        reinterpret_cast<AppThreadDelegate *>(stored_pointer);
    if (delegate)
    {
        delegate->Init();
        //message_loop()->PostTask(FROM_HERE,
        //                         base::Bind(&AppThreadDelegate::InitAsync,
        //                                    // Delegate is expected to exist for the
        //                                    // duration of the thread's lifetime
        //                                    base::Unretained(delegate)));
        base::PostTask(FROM_HERE, {}, base::BindOnce(&AppThreadDelegate::InitAsync,
            // Delegate is expected to exist for the
            // duration of the thread's lifetime
            base::Unretained(delegate)));
    }
}

void AppThreadImpl::Run(base::RunLoop* run_loop)
{
    /*AppThread::ID thread_id;
    if (!GetCurrentThreadIdentifier(&thread_id))
    {
        return Thread::Run(run_loop);
    }*/

    switch (identifier_)
    {
    case AppThread::UI:
        return UIThreadRun(run_loop);
    case AppThread::IO:
        return IOThreadRun(run_loop);
    /*case AppThread::DB:
        return DBThreadRun(run_loop);
    case AppThread::FILE:
        return FileThreadRun(run_loop);*/
    case AppThread::ID_COUNT:
        CHECK(false);  // This shouldn't actually be reached!
        break;
    }
    Thread::Run(run_loop);
}

void AppThreadImpl::CleanUp()
{
    AppThreadGlobals& globals = g_globals.Get();

    using base::subtle::AtomicWord;
    AtomicWord* storage =
        reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
    AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
    AppThreadDelegate* delegate =
        reinterpret_cast<AppThreadDelegate*>(stored_pointer);

    if (delegate)
    {
        delegate->CleanUp();
    }
}

base::PlatformThreadId AppThreadImpl::GetThreadId() const
{
    if (identifier_ != AppThread::UI)
    {
        return base::Thread::GetThreadId();
    }

    return thread_id_;
}


scoped_refptr<base::SingleThreadTaskRunner> AppThreadImpl::task_runner() const
{
    if (identifier_ != AppThread::UI)
    {
        return base::Thread::task_runner();
    }

    DCHECK(task_runner_);
    return task_runner_;
}

void AppThreadImpl::Initialize()
{
    AppThreadGlobals &globals = g_globals.Get();

    base::AutoLock lock(globals.lock);
    DCHECK(identifier_ >= 0 && identifier_ < ID_COUNT);
    DCHECK(globals.threads[identifier_] == NULL);
    globals.threads[identifier_] = this;
}

AppThreadImpl::~AppThreadImpl()
{
    Stop();

    AppThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    globals.threads[identifier_] = NULL;
#ifndef NDEBUG
    // Double check that the threads are ordered correctly in the enumeration.
    for (int i = identifier_ + 1; i < ID_COUNT; ++i)
    {
        DCHECK(!globals.threads[i]) <<
                                    "Threads must be listed in the reverse order that they die";
    }
#endif
}

// static
bool AppThreadImpl::PostTaskHelper(
    AppThread::ID identifier,
    const base::Location &from_here,
    base::OnceClosure task,
    base::TimeDelta delay,
    bool nestable)
{
    DCHECK(identifier >= 0 && identifier < ID_COUNT);

    AppThread::ID current_thread;
    bool target_thread_outlives_current =
        GetCurrentThreadIdentifier(&current_thread) &&
        current_thread >= identifier;

    AppThreadGlobals &globals = g_globals.Get();
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


// static
bool AppThread::PostBlockingPoolTask(
    const base::Location& from_here,
    base::OnceClosure task)
{
    if (base::ThreadPoolInstance::Get())
    {
        return base::PostTask(from_here, { base::ThreadPool(),  base::MayBlock() }, std::move(task));
    }
    return false;
}

bool AppThread::PostBlockingPoolTaskAndReply(
    const base::Location &from_here,
    base::OnceClosure task,
    base::OnceClosure reply)
{
    if (base::ThreadPoolInstance::Get())
    {
        return base::PostTaskAndReply(from_here, { base::ThreadPool(),  base::MayBlock() }, std::move(task), std::move(reply));
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

    AppThreadGlobals &globals = g_globals.Get();
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
    AppThreadGlobals &globals = g_globals.Get();
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

    AppThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] &&
           globals.threads[identifier]->task_runner();
}

// static
bool AppThread::PostTask(ID identifier,
                              const base::Location &from_here,
                              base::OnceClosure task)
{
    return AppThreadImpl::PostTaskHelper(
               identifier, from_here, std::move(task), base::TimeDelta(), true);
}

// static
bool AppThread::PostDelayedTask(ID identifier,
                                     const base::Location &from_here,
                                     base::OnceClosure task,
                                     base::TimeDelta delay)
{
    return AppThreadImpl::PostTaskHelper(
               identifier, from_here, std::move(task), delay, true);
}

// static
bool AppThread::PostNonNestableTask(
    ID identifier,
    const base::Location &from_here,
    base::OnceClosure task)
{
    return AppThreadImpl::PostTaskHelper(
               identifier, from_here, std::move(task), base::TimeDelta(), false);
}

// static
bool AppThread::PostNonNestableDelayedTask(
    ID identifier,
    const base::Location &from_here,
    base::OnceClosure task,
    base::TimeDelta delay)
{
    return AppThreadImpl::PostTaskHelper(
               identifier, from_here, std::move(task), delay, false);
}

// static
bool AppThread::PostTaskAndReply(
    ID identifier,
    const base::Location &from_here,
    base::OnceClosure task,
    base::OnceClosure reply)
{
    return GetMessageLoopProxyForThread(identifier)->PostTaskAndReply(from_here,
        std::move(task),
        std::move(reply));
}

// static
bool AppThread::GetCurrentThreadIdentifier(ID *identifier)
{
    if (!g_globals.IsCreated())
    {
        return false;
    }

    AppThreadGlobals &globals = g_globals.Get();
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
AppThread::GetMessageLoopProxyForThread(ID identifier)
{
    return base::ThreadTaskRunnerHandle::Get();
}

// static
void AppThread::SetDelegate(ID identifier,
                            AppThreadDelegate *delegate)
{
    using base::subtle::AtomicWord;
    AppThreadGlobals &globals = g_globals.Get();
    AtomicWord *storage = reinterpret_cast<AtomicWord *>(
                              &globals.thread_delegates[identifier]);
    AtomicWord old_pointer = base::subtle::NoBarrier_AtomicExchange(
                                 storage, reinterpret_cast<AtomicWord>(delegate));

    // This catches registration when previously registered.
    DCHECK(!delegate || !old_pointer);
}
