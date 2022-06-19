#include "iocp.h"

#include "logging/logging.h"

namespace mctm
{
    IOCP::IOCP(unsigned int thread_count)
    {
        port_.SetHandle(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, thread_count));
        DCHECK(port_);
    }

    IOCP::~IOCP()
    {
    }
    
    bool IOCP::RegisterIOHandle(HANDLE handle, ULONG_PTR key)
    {
        if (!port_)
        {
            return false;
        }

        HANDLE port = ::CreateIoCompletionPort(handle, port_, key, 0);
        DCHECK(port);
        return !!port;
    }

    bool IOCP::GetIOItem(DWORD timeout, IOItem* item)
    {
        memset(item, 0, sizeof(*item));
        OVERLAPPED* overlapped = nullptr;
        if (!::GetQueuedCompletionStatus(port_, &item->bytes_transfered, &item->key, &overlapped, timeout))
        {
            if (!overlapped)
            {
                return false;  // Nothing in the queue.
            }

            item->error = ::GetLastError();
            item->bytes_transfered = 0;
        }

        item->overlapped = overlapped;
        return true;
    }

}