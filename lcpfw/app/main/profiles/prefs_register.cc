#include "prefs_register.h"

#include "main/app_main_process_impl.h"

namespace lcpfw {

void RegisterGlobalProfilePrefs(PrefRegistrySimple* registry)
{
    AppMainProcessImpl::RegisterGlobalPrefs(registry);
}

void RegisterUserProfilePrefs(PrefRegistrySimple* registry)
{
    AppMainProcessImpl::RegisterUserPrefs(registry);
}

}
