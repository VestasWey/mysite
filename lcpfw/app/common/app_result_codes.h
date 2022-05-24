#pragma once

namespace lcpfw
{
    enum ResultCode : int {
        ResultCodeNormalExit = 0,

        ResultCodeRestartApp,

        ResultCodeLoginCancelled,

        ResultCodeErrorOccurred,

        ResultCodeMissingData
    };
}
