#pragma once

#include "common/profiles/profile.h"

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_registry_simple.h"

namespace base
{
    class SequencedTaskRunner;
}

class ProfileImpl
    : public Profile
{
public:
    base::FilePath GetPath() override;
    base::FilePath GetPath() const override;

    PrefService* GetPrefs() override;
    const PrefService* GetPrefs() const override;

protected:
    ProfileImpl(const base::FilePath& path,
        Delegate* delegate,
        ProfilePrefsRegisterFunc prefs_registerar,
        scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

    ~ProfileImpl() override;

private:
    friend class Profile;
    //friend struct std::default_delete<ProfileImpl>;

    void DoFinalInit();

    void OnPrefsLoaded(bool success);

    // PrefServiceFactory callback
    void HandleReadError(PersistentPrefStore::PrefReadError error);

    bool IsNewProfile();

    base::FilePath GetPrefFilePath();

private:
    base::FilePath file_path_;

    scoped_refptr<PrefRegistrySimple> pref_registry_;
    std::unique_ptr<PrefService> prefs_;

    Profile::Delegate *delegate_;

    PersistentPrefStore::PrefReadError pref_read_error_;

    DISALLOW_COPY_AND_ASSIGN(ProfileImpl);
};
