#include "time_util.h"

#include <mutex>

#include <windows.h>
#include <timeapi.h>

namespace
{
    //std::lock_guard<std.recursive_mutex>
    std::recursive_mutex g_mutex;
    DWORD g_last_seen_now = 0;
    __int64 g_rollover_ms = 0;

    mctm::TimeDelta RolloverProtectedNow()
    {
        std::lock_guard<std::recursive_mutex> locked(g_mutex);
        // We should hold the lock while calling tick_function to make sure that
        // we keep last_seen_now stay correctly in sync.
        DWORD now = ::timeGetTime();
        // 当前的毫秒数小于上次拿到的毫秒数则说明系统的运行时间已经超过timeGetTime返回值的有效范围了
        // 即timeGetTime一个轮回大约49.71天，那么运行总时长就自增一个轮回
        if (now < g_last_seen_now)
        {
            g_rollover_ms += 0x100000000I64;  // ~49.7 days.
        }
        g_last_seen_now = now;
        return mctm::TimeDelta::FromMilliseconds(now + g_rollover_ms);
    }
}

namespace mctm
{
    // TimeDelta
    TimeDelta::TimeDelta()
    {
    }

    TimeDelta::TimeDelta(__int64 ms)
        : delta_in_us_(ms)
    {
    }

    TimeDelta::~TimeDelta()
    {
    }

    TimeDelta TimeDelta::FromMilliseconds(__int64 ms)
    {
        return TimeDelta(ms * Time::kMicrosecondsPerMillisecond);
    }

    double TimeDelta::InMillisecondsF() const
    {
        return static_cast<double>(delta_in_us_) / Time::kMicrosecondsPerMillisecond;
    }

    __int64 TimeDelta::InMilliseconds() const
    {
        return delta_in_us_ / Time::kMicrosecondsPerMillisecond;
    }


    // TimeTicks
    TimeTicks::TimeTicks()
    {
    }

    TimeTicks::TimeTicks(__int64 us)
        : ticks_in_us_(us)
    {
    }

    TimeTicks::~TimeTicks()
    {
    }

    TimeTicks TimeTicks::Now()
    {
        return TimeTicks() + RolloverProtectedNow();
    }

    bool TimeTicks::is_null() const
    {
        return ticks_in_us_ == 0;
    }

}