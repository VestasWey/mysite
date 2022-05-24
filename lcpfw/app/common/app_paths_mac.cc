
#include "base/path_service.h"
#include "base/files/file_util.h"
#include "base/mac/bundle_locations.h"
#include "base/strings/utf_string_conversions.h"

#include "public/common/app_paths_internal.h"
#include "public/common/app_constants.h"


namespace lcpfw
{
    using namespace base;

    namespace
    {
        bool GetLocalUserDataDirectory(base::FilePath *result)
        {
            base::FilePath hd = base::GetHomeDir();
            hd = hd.Append("Library/Application Support");
            *result = hd;
            return true;
        }
    }

    bool GetDefaultUserDataDirectory(base::FilePath *result)
    {
        GetLocalUserDataDirectory(result);
        *result = result->Append(lcpfw::kAppFullName);
        *result = result->Append(lcpfw::kUserDataDirname);
        return true;
    }

    void GetUserCacheDirectory(const base::FilePath &profile_dir,
                               base::FilePath *result)
    {
        *result = profile_dir;
    }

    bool GetUserDocumentsDirectory(base::FilePath *result)
    {
        *result = base::GetHomeDir().Append("Documents");
        return true;
    }

    bool GetUserDownloadsDirectorySafe(base::FilePath *result)
    {
        *result = base::GetHomeDir().Append("Downloads");
        return true;
    }

    bool GetUserDownloadsDirectory(base::FilePath *result)
    {
        *result = base::GetHomeDir().Append("Downloads");
        return true;
    }

    bool GetUserMusicDirectory(base::FilePath *result)
    {
        *result = base::GetHomeDir().Append("Music");
        return true;
    }

    bool GetUserPicturesDirectory(base::FilePath *result)
    {
        *result = base::GetHomeDir().Append("Pictures");
        return true;
    }

    bool GetUserVideosDirectory(base::FilePath *result)
    {
        *result = base::GetHomeDir().Append("Movies");
        return true;
    }

    bool ProcessNeedsProfileDir(const std::string &process_type)
    {
        return process_type.empty();
    }

    bool GetUserAccountConfigDirectory(const std::string &account_name,
                                           base::FilePath *result)
    {
        if (!GetDefaultUserDataDirectory(result))
            return false;

        *result = result->Append(account_name);
        return true;
    }

    base::FilePath QueryInstalledDirectoryFromRegistry()
    {
        NOTREACHED();
        return base::FilePath();
    }

}

