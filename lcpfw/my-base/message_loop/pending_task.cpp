#include "pending_task.h"


namespace mctm
{

    PendingTask::PendingTask()
    {
    }

    PendingTask::PendingTask(const Location & posted_from, const Closure& task, 
        TimeTicks delayed_run_time, bool nestable)
    {
        this->posted_from = posted_from;
        this->task = task;
        this->delayed_run_time = delayed_run_time;
        this->time_posted = TimeTicks::Now();
        this->nestable = nestable;
    }

    PendingTask::~PendingTask()
    {
    }

    bool PendingTask::operator<(const PendingTask& other) const
    {
        // Since the top of a priority queue is defined as the "greatest" element, we
        // need to invert the comparison here.  We want the smaller time to be at the
        // top of the heap.

        if (delayed_run_time < other.delayed_run_time)
            return false;

        if (delayed_run_time > other.delayed_run_time)
            return true;

        // If the times happen to match, then we use the sequence number to decide.
        // Compare the difference to support integer roll-over.
        return (sequence_num - other.sequence_num) > 0;
    }

}