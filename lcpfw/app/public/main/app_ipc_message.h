#pragma once

namespace ipc_message
{
    const wchar_t kInstanceMutexLiveHimeOBSPlugins[] = L"{245FDFA3-3016-47A6-9814-64DEE01DC36A}";
    const char kIPCChannelLiveHimeOBSPlugins[] = "livehime_obs_plugin_ipc_channel";

    enum LivehimeOBSPluginIPCMessageType : unsigned int
    {
        IPC_MSG_BEGIN = 100,

        // test[
        IPC_LIVEHIME_TO_OBS,
        IPC_OBS_TO_LIVEHIME,
        IPC_BOTHWAY,
        // ]

        IPC_OBS_TO_LIVEHIME_START_LIVE,
        IPC_LIVEHIME_TO_OBS_STREAM_SETTINGS,
        IPC_LIVEHIME_TO_OBS_STOP_STREAMING,

        IPC_MSG_END,
    };

}
