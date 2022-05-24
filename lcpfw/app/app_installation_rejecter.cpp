#include "app_installation_rejecter.h"

#ifdef _MSC_VER
#include <windows.h>

#include "base/win/scoped_handle.h"
#endif

namespace
{
#ifdef _MSC_VER
    bool InstallationRejectWin()
    {
        static const wchar_t kInstallerMutex[] = L"{915299E6-E1EF-4328-B0C3-0A58D8F54AC7}";
        base::win::ScopedHandle instance_mutex;
        instance_mutex.Set(::OpenMutexW(SYNCHRONIZE, FALSE, kInstallerMutex));
        if (instance_mutex.IsValid()) {
            PLOG(WARNING) << "Installer is running!";
            return true;
        }

        if (::GetLastError() == ERROR_FILE_NOT_FOUND) {
            return false;
        }

        return false;
    }
#else
    bool InstallationRejectMac()
    {
        return false;
    }
#endif
}

bool AppInstallationRejecter::Reject()
{
#ifdef _MSC_VER
    return InstallationRejectWin();
#else
    return InstallationRejectMac();
#endif

    return false;
}
