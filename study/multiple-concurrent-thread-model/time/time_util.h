#pragma once

namespace mctm
{
    class TimeTicks;
    // 时间间隔
    class TimeDelta
    {
    public:
        TimeDelta();
        ~TimeDelta();

        static TimeDelta FromMilliseconds(__int64 ms);

        double InMillisecondsF() const;
        __int64 InMilliseconds() const;

        // Comparison operators.
        bool operator==(TimeDelta other) const
        {
            return delta_in_us_ == other.delta_in_us_;
        }
        bool operator!=(TimeDelta other) const
        {
            return delta_in_us_ != other.delta_in_us_;
        }
        bool operator<(TimeDelta other) const
        {
            return delta_in_us_ < other.delta_in_us_;
        }
        bool operator<=(TimeDelta other) const
        {
            return delta_in_us_ <= other.delta_in_us_;
        }
        bool operator>(TimeDelta other) const
        {
            return delta_in_us_ > other.delta_in_us_;
        }
        bool operator>=(TimeDelta other) const
        {
            return delta_in_us_ >= other.delta_in_us_;
        }

    private:
        explicit TimeDelta(__int64 ms);

    private:
        friend class TimeTicks;
        // Delta in microseconds.
        __int64 delta_in_us_ = 0;
    };

    // 系统启动以来的毫秒数
    class TimeTicks
    {
    public:
        TimeTicks();
        ~TimeTicks();

        static TimeTicks Now();

        bool is_null() const;

        TimeTicks& operator=(TimeTicks other)
        {
            ticks_in_us_ = other.ticks_in_us_;
            return *this;
        }
        TimeDelta operator-(TimeTicks other) const
        {
            return TimeDelta(ticks_in_us_ - other.ticks_in_us_);
        }

        // Modify by some time delta.
        TimeTicks& operator+=(TimeDelta delta)
        {
            ticks_in_us_ += delta.delta_in_us_;
            return *this;
        }
        TimeTicks& operator-=(TimeDelta delta)
        {
            ticks_in_us_ -= delta.delta_in_us_;
            return *this;
        }
        TimeTicks operator+(TimeDelta delta) const
        {
            return TimeTicks(ticks_in_us_ + delta.delta_in_us_);
        }
        TimeTicks operator-(TimeDelta delta) const
        {
            return TimeTicks(ticks_in_us_ - delta.delta_in_us_);
        }

        // Comparison operators
        bool operator==(TimeTicks other) const
        {
            return ticks_in_us_ == other.ticks_in_us_;
        }
        bool operator!=(TimeTicks other) const
        {
            return ticks_in_us_ != other.ticks_in_us_;
        }
        bool operator<(TimeTicks other) const
        {
            return ticks_in_us_ < other.ticks_in_us_;
        }
        bool operator<=(TimeTicks other) const
        {
            return ticks_in_us_ <= other.ticks_in_us_;
        }
        bool operator>(TimeTicks other) const
        {
            return ticks_in_us_ > other.ticks_in_us_;
        }
        bool operator>=(TimeTicks other) const
        {
            return ticks_in_us_ >= other.ticks_in_us_;
        }

    private:
        explicit TimeTicks(__int64 ms);

    private:
        // Delta in microseconds.
        __int64 ticks_in_us_ = 0;
    };

    class Time
    {
    public:
        static const __int64 kMillisecondsPerSecond = 1000;
        static const __int64 kMicrosecondsPerMillisecond = 1000;
        static const __int64 kMicrosecondsPerSecond = kMicrosecondsPerMillisecond *
            kMillisecondsPerSecond;
        static const __int64 kMicrosecondsPerMinute = kMicrosecondsPerSecond * 60;
        static const __int64 kMicrosecondsPerHour = kMicrosecondsPerMinute * 60;
        static const __int64 kMicrosecondsPerDay = kMicrosecondsPerHour * 24;
        static const __int64 kMicrosecondsPerWeek = kMicrosecondsPerDay * 7;
        static const __int64 kNanosecondsPerMicrosecond = 1000;
        static const __int64 kNanosecondsPerSecond = kNanosecondsPerMicrosecond *
            kMicrosecondsPerSecond;
    };
}
