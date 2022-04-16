#include "ipc_message.h"

#include <atomic>
#include <processthreadsapi.h>

#include "logging/logging.h"

namespace
{
    std::atomic_int g_ref_num = 0;

    // Create a reference number for identifying IPC messages in traces. The return
    // values has the reference number stored in the upper 24 bits, leaving the low
    // 8 bits set to 0 for use as flags.
    inline unsigned int GetRefNumUpper24()
    {
        static int pid = ::GetCurrentProcessId();
        int count = g_ref_num++;
        // The 24 bit hash is composed of 14 bits of the count and 10 bits of the
        // Process ID. With the current trace event buffer cap, the 14-bit count did
        // not appear to wrap during a trace. Note that it is not a big deal if
        // collisions occur, as this is only used for debugging and trace analysis.
        return ((pid << 22) | (count & 0x3fff)) << 8;
    }
}

namespace mctm
{
    IPCMessage::IPCMessage(int routing_id, unsigned int type, PriorityValue priority)
        : Pickle(sizeof(Header))
    {
        header()->routing = routing_id;
        header()->type = type;
        DCHECK((priority & 0xffffff00) == 0);
        header()->flags = priority | GetRefNumUpper24();
    }

    IPCMessage::IPCMessage(const char* data, int data_len)
        : Pickle(data, data_len)
    {
    }

    IPCMessage::~IPCMessage()
    {
    }

}