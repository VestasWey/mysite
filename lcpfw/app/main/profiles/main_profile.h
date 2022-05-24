#pragma once

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_registry_simple.h"

#include "common/profiles/profile_impl.h"

//class NetPrefObserver;
//class PrefServiceSyncable;
//class SSLConfigServiceManager;

class MainProfile
    : public ProfileImpl
{
public:
    static std::unique_ptr<Profile> CreateGlobalProfile(const base::FilePath& path,
        Delegate* delegate,
        scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

    static std::unique_ptr<Profile> CreateProfile(const base::FilePath& path,
        Delegate* delegate,
        scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

private:
    DISALLOW_COPY_AND_ASSIGN(MainProfile);
};
