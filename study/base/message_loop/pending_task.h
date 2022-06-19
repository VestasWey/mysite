#pragma once
#include <queue>

#include "functional/callback.h"
#include "message_loop/location.h"
#include "time/time_util.h"

namespace mctm
{
    struct PendingTask
    {
        PendingTask();
        PendingTask(const Location& posted_from,
            const Closure& task,
            TimeTicks delayed_run_time,
            bool nestable);
        ~PendingTask();

        // Used to support sorting.
        bool operator<(const PendingTask& other) const;

        Closure task;
        // The site this PendingTask was posted from.
        Location posted_from;
        // Secondary sort key for run time.
        int sequence_num = -1;
        // OK to dispatch from a nested loop.
        bool nestable = true;

        // Time when the related task was posted.
        TimeTicks time_posted;
        // The time when the task should be run.
        TimeTicks delayed_run_time;
    };

    using TaskQueue = std::queue<PendingTask>;
    using DelayedTaskQueue = std::priority_queue<PendingTask>;
}
