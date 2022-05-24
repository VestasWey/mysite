// Copy from base/task/post_task.h file.

#pragma once

#include <memory>
#include <utility>

#include "base/base_export.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/post_task_and_reply_with_result_internal.h"
#include "base/single_thread_task_runner.h"
#include "base/task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"


// 接口参照 base/task/post_task.h，不同的的地方是，base/task/post_task.h 中的任务投递要求每个模块必须
// 先通过 base::RegisterTaskExecutor 对指定特性的TaskRunner进行全局注册设置，使用方才能根据特性属性拿到对应的
// TaskRunner才能投递任务，而这个注册操作可能来自任何线程。
// 从 base/task/post_task.h 的注释也可以看到，那里提供的便携式的任务投递接口是配合线程池ThreadPool使用的，
// 而如果我们只是希望把任务投递到当前线程的话，需要类似这样的操作：
//    DCHECK(base::ThreadTaskRunnerHandle::Get());
//    base::ThreadTaskRunnerHandle::Get()->PostXxxxTask();
// 为了减少这样的实例判断，这里只是进行了一些简单的封装
namespace lcpfw {

    // for special thread ===============================================================
    typedef int ThreadID;

    void RegisterAppThread(ThreadID id, scoped_refptr<base::SingleThreadTaskRunner> task_runner);
    scoped_refptr<base::SingleThreadTaskRunner> AppThreadTaskRunnerHandle(ThreadID id);
    void UnregisterAppThread(ThreadID id);
    bool BelongsToCurrentThread(ThreadID id);

    bool PostTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task);

    bool PostDelayedTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay);

    bool PostTaskAndReply(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::OnceClosure reply);

    bool PostNonNestableTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task);

    bool PostNonNestableDelayedTask(ThreadID identifier,
        const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay);

    template <typename TaskReturnType, typename ReplyArgType>
    bool PostTaskAndReplyWithResult(ThreadID identifier,
        const base::Location& from_here,
        base::OnceCallback<TaskReturnType()> task,
        base::OnceCallback<void(ReplyArgType)> reply) {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->PostTaskAndReplyWithResult(from_here, std::move(task), std::move(reply));
    }

    template <class T>
    bool DeleteSoon(ThreadID identifier,
        const base::Location& from_here,
        const T* object) {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        return task_runner->DeleteSoon(from_here, object);
    }

    template <class T>
    bool DeleteSoon(ThreadID identifier,
        const base::Location& from_here,
        std::unique_ptr<T> object) {
        return DeleteSoon(identifier, from_here, object.release());
    }

    template <class T>
    void ReleaseSoon(ThreadID identifier,
        const base::Location& from_here,
        scoped_refptr<T>&& object) {
        auto task_runner = AppThreadTaskRunnerHandle(identifier);
        DCHECK(task_runner);
        task_runner->ReleaseSoon(from_here, std::move(object));
    }

    // for current thread ===============================================================
    bool PostTask(const base::Location& from_here,
        base::OnceClosure task);

    bool PostDelayedTask(const base::Location& from_here,
        base::OnceClosure task,
        base::TimeDelta delay);

    bool PostTaskAndReply(const base::Location& from_here,
        base::OnceClosure task,
        base::OnceClosure reply);

    template <typename TaskReturnType, typename ReplyArgType>
    bool PostTaskAndReplyWithResult(const base::Location& from_here,
        base::OnceCallback<TaskReturnType()> task,
        base::OnceCallback<void(ReplyArgType)> reply) {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->PostTaskAndReplyWithResult(from_here, std::move(task), std::move(reply));
    }

    template <class T>
    bool DeleteSoon(const base::Location& from_here,
        const T* object) {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        return task_runner->DeleteSoon(from_here, object);
    }

    template <class T>
    bool DeleteSoon(const base::Location& from_here,
        std::unique_ptr<T> object) {
        return DeleteSoon(from_here, object.release());
    }

    template <class T>
    void ReleaseSoon(const base::Location& from_here,
        scoped_refptr<T>&& object) {
        auto task_runner = base::ThreadTaskRunnerHandle::Get();
        DCHECK(task_runner);
        task_runner->ReleaseSoon(from_here, std::move(object));
    }

}  // namespace lcpfw

