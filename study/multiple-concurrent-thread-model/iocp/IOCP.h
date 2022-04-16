#pragma once
#include <windows.h>

#include "data_encapsulation/smart_pointer.h"

namespace mctm
{
    class IOCP
    {
    public:
        struct IOItem
        {
            ULONG_PTR key = 0;
            OVERLAPPED* overlapped = nullptr;
            DWORD bytes_transfered = 0;
            DWORD error = 0;
        };

    public:
        explicit IOCP(unsigned int thread_count);
        ~IOCP();

        /************************************
        * Method:    RegisterIOHandler
        * FullName:  mctm::IOCP::RegisterIOHandler
        * Access:    public 
        * Returns:   bool
        * Parameter: HANDLE pipe_or_file
        * Parameter: IOHandler * handler
        * Remarks: 上层应该自己保证HANDLE和IOHandler的唯一对应，因为该函数将以handler作为pipe_or_file
        *          绑定到iocp的标识，如果不做唯一对应，那么在OnIOCompleted回调时上层就没法判断这个I/O完成
        *          操作是属于哪个HANDLE的。
        *          典型的用法应该是，比如一个IPCChannel实例封装并维护一个hPipeHandle，IPCChannel继承IOHandler，
        *          则在该IPCChannel::OnIOCompleted就是属于hPipeHandle的异步回调
        ************************************/
        bool RegisterIOHandle(HANDLE handle, ULONG_PTR key);

        bool GetIOItem(DWORD timeout, IOItem* item);

        operator HANDLE() const
        {
            return port_;
        }

    private:
        ScopedHandle port_;
    };
}

