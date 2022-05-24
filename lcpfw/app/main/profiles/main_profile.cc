#include "main_profile.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/strings/string_util.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_value_store.h"

#include "common/app_constants.h"
#include "main/profiles/prefs_register.h"


std::unique_ptr<Profile> MainProfile::CreateGlobalProfile(const base::FilePath& path,
    Delegate* delegate,
    scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner)
{
    return Profile::CreateProfile(
        path.Append(lcpfw::kPreferencesFilename),
        delegate,
        lcpfw::RegisterGlobalProfilePrefs,
        sequenced_task_runner);
}

std::unique_ptr<Profile> MainProfile::CreateProfile(const base::FilePath& path,
    Delegate* delegate,
    scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner)
{
    return Profile::CreateProfile(
        path.Append(lcpfw::kPreferencesFilename),
        delegate,
        lcpfw::RegisterUserProfilePrefs,
        sequenced_task_runner);
}
