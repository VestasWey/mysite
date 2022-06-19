#include "semaphore.h"

#include <handleapi.h>
#include <synchapi.h>
#include <WinBase.h>


namespace mctm
{
    Semaphore::Semaphore(long initial, long maxcount, const wchar_t* name/* = nullptr*/)
    {
        sem_ = ::CreateSemaphore(nullptr, initial, maxcount, name);
        if (!sem_)
        {
            //DWORD dw = GetLastError();
            //ERROR_INVALID_HANDLE
        }
    }

    Semaphore::~Semaphore()
    {
        Close();
    }

    bool Semaphore::Open(const wchar_t* name)
    {
        if (!sem_)
        {
            sem_ = ::OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, name);
            return true;
        }
        return false;
    }

    void Semaphore::Signal()
    {
        if (sem_)
        {
            ::ReleaseSemaphore(sem_, 1, nullptr);
        }
    }

    bool Semaphore::Wait() const
    {
        return TimedWait(INFINITE);
    }

    bool Semaphore::TimedWait(DWORD dwMilliseconds) const
    {
        if (sem_)
        {
            DWORD dw = ::WaitForSingleObject(sem_, dwMilliseconds);
            return (dw == WAIT_OBJECT_0);
        }
        return false;
    }

    void Semaphore::Close()
    {
        if (sem_)
        {
            ::CloseHandle(sem_);
            sem_ = nullptr;
        }
    }

}
