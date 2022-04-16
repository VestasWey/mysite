#pragma once

#include "location.h"
#include "message_pump.h"

#include "data_encapsulation/smart_pointer.h"
#include "functional/callback.h"
#include "incoming_task_queue.h"
#include "time/time_util.h"
#include <functional>

namespace
{
    template <class T>
    class DeleteHelper
    {
    public:
        static void DoDelete(const void* object)
        {
            if (object)
            {
                delete reinterpret_cast<const T*>(object);
            }
        }

        //DISALLOW_COPY_AND_ASSIGN(DeleteHelper);
    };

    template <class T>
    class ReleaseHelper
    {
    public:
        static void DoRelease(const void* object)
        {
            if (object)
            {
                reinterpret_cast<const T*>(object)->Release();
            }
        }

        //DISALLOW_COPY_AND_ASSIGN(ReleaseHelper);
    };
}

namespace mctm
{
    class RunLoop;
    class MessageLoop
        : MessagePump::Delegate
        , public std::enable_shared_from_this<MessageLoop>
    {
    public:
        using CheckExtensionalLoopSignalHandler = std::function<bool()>;

        enum class Type
        {
            TYPE_DEFAULT,
            TYPE_UI,
            TYPE_IO,
        };

        explicit MessageLoop(Type type);
        ~MessageLoop();

        static MessageLoop* current();

        Type type() const { return type_; }

        void PostTask(const Location& from_here,
            const Closure& task);

        void PostDelayedTask(const Location& from_here,
            const Closure& task,
            TimeDelta delay);

        void PostIdleTask(const Location& from_here,
            const Closure& task);

        void PostTaskAndReply(
            const Location& from_here,
            const Closure& task,
            const Closure& reply);

        template <class T>
        void DeleteSoon(const Location& from_here, const T* object)
        {
            if (!object)
            {
                return;
            }

            PostIdleTask(from_here, Bind(DeleteHelper<T>::DoDelete, object));
        }

        template <class T>
        void ReleaseSoon(const Location& from_here, const T* object)
        {
            if (!object)
            {
                return;
            }

            PostIdleTask(from_here, Bind(ReleaseHelper<T>::DoRelease, object));
        }

        // quit current when idle
        void Quit();

        // Returns true if we are currently running a nested message loop.
        bool IsNested();

        void set_check_extensional_loop_signal_handler(
            CheckExtensionalLoopSignalHandler check_extensional_loop_signal_handler)
        {
            check_extensional_loop_signal_handler_ = check_extensional_loop_signal_handler;
        }

    protected:
        // MessagePump::Delegate
        bool ShouldQuitCurrentLoop() override;
        void QuitCurrentLoopNow() override;
        bool CheckExtensionalLoopSignal() override;
        bool DoWork() override;
        bool DoDelayedWork(TimeTicks* next_delayed_work_time) override;
        bool DoIdleWord() override;

    private:
        void DoRunLoop();
        void ReloadWorkQueue();
        bool DeferOrRunPendingTask(const PendingTask& pending_task);
        void AddToDelayedWorkQueue(const PendingTask& pending_task);
        bool ProcessNextDelayedNonNestableTask();
        void RunTask(const PendingTask& pending_task);
        void ScheduleWork(bool pre_task_queue_status_was_empty);
        void QuitWhenIdle();

        // call by RunLoop
        void set_run_loop(RunLoop* run_loop);
        RunLoop* current_run_loop();

    protected:
        ScopedMessagePump pump_;

    private:
        friend class RunLoop;
        friend class IncomingTaskQueue;

        Type type_ = Type::TYPE_DEFAULT;
        RunLoop* current_run_loop_ = nullptr;
        IncomingTaskQueue incoming_task_queue_;
        TaskQueue work_queue_;
        TaskQueue deferred_non_nestable_work_queue_;
        DelayedTaskQueue delayed_work_queue_;
        TimeTicks recent_time_;
        CheckExtensionalLoopSignalHandler check_extensional_loop_signal_handler_ = nullptr;
    };

    class MessageLoopForUI : public MessageLoop
    {
    public:
        static MessageLoopForUI* current();

    protected:
        MessagePumpForUI* pump_ui();
    };

    class MessageLoopForIO : public MessageLoop
    {
    public:
        static MessageLoopForIO* current();

        bool RegisterIOHandler(HANDLE file_handle, MessagePumpForIO::IOHandler* handler);
        bool RegisterJobObject(HANDLE job_handle, MessagePumpForIO::IOHandler* handler);
        bool WaitForIOCompletion(DWORD timeout, MessagePumpForIO::IOHandler* filter);

    protected:
        MessagePumpForIO* pump_io();
    };
}

