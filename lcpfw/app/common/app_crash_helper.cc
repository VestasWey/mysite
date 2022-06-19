#include "common/app_crash_helper.h"

#if OS_WIN
#include <handleapi.h>
#include <synchapi.h>
#include <winbase.h>
#endif

#include "base/strings/stringprintf.h"

#include "common/app_constants.h"

namespace {

    const DWORD kWaitCrashServerReadyTimeInterval = 250;    // in milliseconds

}

std::string GetCrashInfoFileName(base::ProcessId client_pid)
{
    return base::StringPrintf("crashinfo_%u", client_pid);
}

std::string GetCrashLogFileName(base::ProcessId client_pid)
{
    return base::StringPrintf("crashlog_%u", client_pid);
}

bool WaitForCrashServerReady()
{
#if OS_WIN
    HANDLE evt = ::OpenEventW(SYNCHRONIZE, false, lcpfw::kExceptionHandlerReadyEventName);
    if (!evt)
    {
        return false;
    }

    DWORD ret = ::WaitForSingleObject(evt, kWaitCrashServerReadyTimeInterval);
    ::CloseHandle(evt);

    return ret == WAIT_OBJECT_0;
#endif

    return false;
}
