#pragma once

class PrefRegistrySimple;

namespace lcpfw {

void RegisterGlobalProfilePrefs(PrefRegistrySimple* registry);
void RegisterUserProfilePrefs(PrefRegistrySimple* registry);

}