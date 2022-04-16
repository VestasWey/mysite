#pragma once

#include <mutex>
#include <queue>

#include "message_loop/pending_task.h"
#include "time/time_util.h"

namespace mctm
{
    class MessageLoop;
    class IncomingTaskQueue
    {
    public:
        explicit IncomingTaskQueue(MessageLoop* message_loop);
        ~IncomingTaskQueue();

        bool AddToIncomingQueue(const Location& from_here,
            const Closure& task,
            TimeDelta delay,
            bool nestable);

        void ReloadWorkQueue(TaskQueue* work_queue);

    private:
        MessageLoop* message_loop_;

        std::recursive_mutex incoming_queue_lock_;
        TaskQueue incoming_queue_;
    };

}
