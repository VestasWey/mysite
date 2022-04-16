#include "incoming_task_queue.h"

#include <atomic>

#include "message_loop/message_loop.h"

namespace
{
    mctm::TimeTicks CalculateDelayedRuntime(mctm::TimeDelta delay)
    {
        mctm::TimeTicks delayed_run_time;
        if (delay > mctm::TimeDelta())
        {
            delayed_run_time = mctm::TimeTicks::Now() + delay;

#if defined(OS_WIN)
            if (high_resolution_timer_expiration_.is_null())
            {
                // Windows timers are granular to 15.6ms.  If we only set high-res
                // timers for those under 15.6ms, then a 18ms timer ticks at ~32ms,
                // which as a percentage is pretty inaccurate.  So enable high
                // res timers for any timer which is within 2x of the granularity.
                // This is a tradeoff between accuracy and power management.
                bool needs_high_res_timers = delay.InMilliseconds() <
                    (2 * Time::kMinLowResolutionThresholdMs);
                if (needs_high_res_timers)
                {
                    if (Time::ActivateHighResolutionTimer(true))
                    {
                        high_resolution_timer_expiration_ = TimeTicks::Now() +
                            TimeDelta::FromMilliseconds(
                                MessageLoop::kHighResolutionTimerModeLeaseTimeMs);
                    }
                }
            }
#endif
        }
        else
        {
            //DCHECK_EQ(delay.InMilliseconds(), 0) << "delay should not be negative";
        }

#if defined(OS_WIN)
        if (!high_resolution_timer_expiration_.is_null())
        {
            if (TimeTicks::Now() > high_resolution_timer_expiration_)
            {
                Time::ActivateHighResolutionTimer(false);
                high_resolution_timer_expiration_ = TimeTicks();
            }
        }
#endif

        return delayed_run_time;
    }

    std::atomic_int32_t g_inc_next_sequence_num = 0;
}

namespace mctm
{
    IncomingTaskQueue::IncomingTaskQueue(MessageLoop* message_loop)
        : message_loop_(message_loop)
    {
    }

    IncomingTaskQueue::~IncomingTaskQueue()
    {
    }

    bool IncomingTaskQueue::AddToIncomingQueue(const Location& from_here, 
        const Closure& task, TimeDelta delay, bool nestable)
    {
        bool was_empty = true;
        PendingTask pending_task(from_here, task, CalculateDelayedRuntime(delay), nestable);
        pending_task.sequence_num = g_inc_next_sequence_num++;
        {
            std::lock_guard<std::recursive_mutex> lock(incoming_queue_lock_);
            was_empty = incoming_queue_.empty();
            incoming_queue_.push(std::move(pending_task));
        }
        message_loop_->ScheduleWork(was_empty);
        return true;
    }

    void IncomingTaskQueue::ReloadWorkQueue(TaskQueue* work_queue)
    {
        std::lock_guard<std::recursive_mutex> lock(incoming_queue_lock_);
        incoming_queue_.swap(*work_queue);
    }

}
