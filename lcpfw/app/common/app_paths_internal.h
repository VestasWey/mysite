#pragma once


#include <string>

#include "base/files/file_path.h"
#include "build/build_config.h"


namespace base
{
    class FilePath;
}

namespace lcpfw
{
    bool GetDefaultUserDataDirectory(base::FilePath *result);
    void GetUserCacheDirectory(const base::FilePath &profile_dir, base::FilePath *result);
    bool GetUserDocumentsDirectory(base::FilePath *result);
    bool GetUserDownloadsDirectorySafe(base::FilePath *result);
    bool GetUserDownloadsDirectory(base::FilePath *result);
    bool GetUserMusicDirectory(base::FilePath *result);
    bool GetUserPicturesDirectory(base::FilePath *result);
    bool GetUserVideosDirectory(base::FilePath *result);
    bool ProcessNeedsProfileDir(const std::string &process_type);

    bool GetUserAccountConfigDirectory(const std::string &account_name,
                                           base::FilePath *result);

    base::FilePath QueryInstalledDirectoryFromRegistry();
}

