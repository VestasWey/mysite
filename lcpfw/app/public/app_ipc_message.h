#pragma once

namespace ipc_message
{
    const wchar_t kInstanceMutexXXXX[] = L"{9F9F5B29-B9BC-41FC-A596-AB5E62D054F7}";
    const char kIPCChannelXXXXName[] = "xxxx_ipc_channel";

    enum XXXXIPCMessageType : unsigned int
    {
        IPC_MSG_BEGIN = 100,

        IPC_MSG_END,
    };

}
