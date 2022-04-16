#pragma once
#include "ipc_message.h"

namespace mctm
{
    class IPCChannel;
    class IPCListener
    {
    public:
        virtual bool OnMessageReceived(IPCChannel* channel, const IPCMessage& message) = 0;

        // Called when the channel is connected and we have received the internal
        // Hello message from the peer.
        virtual void OnChannelConnected(IPCChannel* channel, int peer_pid) {}

        // Called when an error is detected that causes the channel to close.
        // This method is not called when a channel is closed normally.
        virtual void OnChannelError(IPCChannel* channel) {}

    protected:
        virtual ~IPCListener() = default;
    };
}