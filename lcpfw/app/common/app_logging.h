#pragma once

#include "base/files/file_path.h"

namespace lcpfw
{
    base::FilePath GetAppLogDirectory();

    /*void BindStandardDevices();
    void RequestConsole();
    bool ConsoleExists();*/

    void InitAppLogging(bool debug_mode);

    // Initializes logging by default mode.
    // This function internally calls its cousin overload.
    void InitAppLogging();

    int64_t GetStartupLogFileOffset();

    std::string GetCurrentLogText();

}

