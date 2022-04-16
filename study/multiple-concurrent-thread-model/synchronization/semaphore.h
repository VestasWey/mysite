#pragma once

#include <wtypes.h>

namespace mctm
{
#ifdef _MSC_VER
    typedef HANDLE sem;
#else
#endif

    class Semaphore
    {
    public:
#ifdef _MSC_VER
        Semaphore(long initial, long maxcount, const wchar_t* name = nullptr);
#else
        Semaphore(bool manual, bool initial);
#endif
        ~Semaphore();

        void Signal();
        bool Wait() const;
        bool TimedWait(DWORD dwMilliseconds) const;
        void Close();

#ifdef _MSC_VER
        bool Open(const wchar_t* name);

        operator HANDLE() const {
            return sem_;
        }
#endif

    private:
        sem sem_ = nullptr;
    };
}


void TestSemaphore();