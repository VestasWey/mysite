#include "profile_impl.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/strings/string_util.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_value_store.h"

#include "common/app_constants.h"


std::unique_ptr<Profile> Profile::CreateProfile(const base::FilePath& path,
    Delegate* delegate,
    ProfilePrefsRegisterFunc prefs_registerar,
    scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner)
{
    return std::unique_ptr<Profile>(
        new ProfileImpl(path,
        delegate,
        prefs_registerar,
        sequenced_task_runner));
}


ProfileImpl::ProfileImpl(const base::FilePath& path,
                         Delegate* delegate,
                         ProfilePrefsRegisterFunc prefs_registerar,
                         scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner)
    : file_path_(path),
    delegate_(delegate),
    pref_registry_(new PrefRegistrySimple()),
    pref_read_error_(PersistentPrefStore::PREF_READ_ERROR_NONE)
{
    DCHECK(!path.empty()) << "Using an empty path will attempt to write " <<
                          "profile files to the root directory!";

    prefs_registerar(pref_registry_.get());

    {
        PrefServiceFactory factory;
        factory.set_async(false);
        factory.SetUserPrefsFile(GetPrefFilePath(), sequenced_task_runner.get());
        factory.set_read_error_callback(base::Bind(&ProfileImpl::HandleReadError, base::Unretained(this)));
        prefs_ = factory.Create(pref_registry_);
    }

    OnPrefsLoaded(!!prefs_);
}

ProfileImpl::~ProfileImpl()
{
}

base::FilePath ProfileImpl::GetPath() const
{
    DCHECK(!file_path_.empty());
    return file_path_.DirName();
}

base::FilePath ProfileImpl::GetPath()
{
    DCHECK(!file_path_.empty());
    return file_path_.DirName();
}

PrefService* ProfileImpl::GetPrefs()
{
    DCHECK(prefs_);
    return prefs_.get();
}

const PrefService* ProfileImpl::GetPrefs() const
{
    DCHECK(prefs_);
    return prefs_.get();
}

void ProfileImpl::OnPrefsLoaded(bool success)
{
    if (!success)
    {
        if (delegate_)
        {
            delegate_->OnProfileCreated(this, false, false);
        }
        return;
    }

    DoFinalInit();
}

void ProfileImpl::DoFinalInit()
{
    if (delegate_)
    {
        delegate_->OnProfileCreated(this, true, IsNewProfile());
    }
}

void ProfileImpl::HandleReadError(PersistentPrefStore::PrefReadError error)
{
    pref_read_error_ = error;
}

base::FilePath ProfileImpl::GetPrefFilePath()
{
    return file_path_;
}

bool ProfileImpl::IsNewProfile()
{
    return GetPrefs()->GetInitializationStatus() ==
         PrefService::INITIALIZATION_STATUS_CREATED_NEW_PREF_STORE;
}
