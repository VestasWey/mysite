#pragma once

#include "data_encapsulation/pickle.h"

namespace mctm
{
    enum SpecialRoutingIDs
    {
        // indicates that we don't have a routing ID yet.
        MSG_ROUTING_NONE = -2,

        // indicates a general message not sent to a particular tab.
        MSG_ROUTING_CONTROL = std::numeric_limits<int>::max(),
    };

    enum
    {
        HELLO_MESSAGE_TYPE = std::numeric_limits<unsigned short>::max()  // Maximum value of message type (uint16),
                                         // to avoid conflicting with normal
                                         // message types, which are enumeration
                                         // constants starting from 0.
    };

    class IPCMessage : public Pickle
    {
    public:
        enum PriorityValue
        {
            PRIORITY_LOW = 1,
            PRIORITY_NORMAL,
            PRIORITY_HIGH
        };

        IPCMessage(int routing_id, unsigned int type, PriorityValue priority);
        IPCMessage(const char* data, int data_len);
        virtual ~IPCMessage();

        static const char* FindNext(const char* range_start, const char* range_end)
        {
            return Pickle::FindNext(sizeof(Header), range_start, range_end);
        }

        unsigned int type() const
        {
            return header()->type;
        }

        int routing_id() const
        {
            return header()->routing;
        }

    protected:
#pragma pack(push, 4)
        struct Header : Pickle::Header
        {
            int routing = 0;  // ID of the view that this message is destined for
            unsigned int type = 0;    // specifies the user-defined message type
            unsigned int flags = 0;   // specifies control flags for the message
        };
#pragma pack(pop)

        Header* header()
        {
            return headerT<Header>();
        }
        const Header* header() const
        {
            return headerT<Header>();
        }

    };
}

