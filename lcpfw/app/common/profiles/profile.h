#pragma once

#include <string>

#include "base/logging.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_refptr.h"
#include "base/sequenced_task_runner.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_registry_simple.h"


class Profile
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        // Called when creation of the profile is finished.
        virtual void OnProfileCreated(Profile* profile,
            bool success,
            bool is_new_profile) = 0;
    };

    Profile() = default;
    virtual ~Profile() = default;

    using ProfilePrefsRegisterFunc = void(*)(PrefRegistrySimple* registry);

    static std::unique_ptr<Profile> CreateProfile(const base::FilePath& path,
        Delegate* delegate,
        ProfilePrefsRegisterFunc prefs_registerar,
        scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

    // Returns the path of the directory where this context's data is stored.
    virtual base::FilePath GetPath() = 0;
    virtual base::FilePath GetPath() const = 0;

    // Retrieves a pointer to the PrefService that manages the
    // preferences for this user profile.
    virtual PrefService* GetPrefs() = 0;
    virtual const PrefService* GetPrefs() const = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(Profile);
};

