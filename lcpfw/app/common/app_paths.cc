#include "common/app_paths.h"

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"

#include "common/app_constants.h"
#include "common/app_paths_internal.h"

namespace lcpfw
{
    using namespace base;

    bool PathProvider(int key, base::FilePath* result)
    {
        switch (key)
        {
        case lcpfw::DIR_APP:
            return PathService::Get(base::DIR_MODULE, result);

        case lcpfw::DIR_LOGS:
            return PathService::Get(lcpfw::DIR_USER_DATA, result);

        case lcpfw::FILE_RESOURCE_MODULE:
            return PathService::Get(base::FILE_MODULE, result);

        default:
            break;
        }

        bool create_dir = false;
        base::FilePath path;
        switch (key)
        {
        case lcpfw::DIR_USER_DATA:
            if (!GetDefaultUserDataDirectory(&path))
            {
                NOTREACHED();
                return false;
            }

            create_dir = true;
            break;

        case lcpfw::DIR_USER_DOCUMENTS:
            if (!GetUserDocumentsDirectory(&path))
            {
                return false;
            }

            create_dir = true;
            break;

        case lcpfw::DIR_USER_MUSIC:
            if (!GetUserMusicDirectory(&path))
            {
                return false;
            }

            break;

        case lcpfw::DIR_USER_PICTURES:
            if (!GetUserPicturesDirectory(&path))
            {
                return false;
            }

            break;

        case lcpfw::DIR_USER_VIDEOS:
            if (!GetUserVideosDirectory(&path))
            {
                return false;
            }

            break;

        case lcpfw::DIR_DEFAULT_DOWNLOADS_SAFE:
            if (!GetUserDownloadsDirectorySafe(&path))
            {
                return false;
            }

            break;

        case lcpfw::DIR_DEFAULT_DOWNLOADS:
            if (!GetUserDownloadsDirectory(&path))
            {
                return false;
            }

            break;

        case lcpfw::DIR_CRASH_DUMPS:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Crash Reports"));
            create_dir = true;
            break;

        case lcpfw::DIR_RESOURCES:
            if (!PathService::Get(lcpfw::DIR_APP, &path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("resources"));
            break;

        case lcpfw::DIR_INSPECTOR:
            if (!PathService::Get(lcpfw::DIR_RESOURCES, &path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("inspector"));
            break;

        case lcpfw::DIR_APP_DICTIONARIES:
            if (!PathService::Get(base::DIR_EXE, &path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Dictionaries"));
            create_dir = true;
            break;

        case lcpfw::DIR_APP_UPDATE:
            if (!PathService::Get(base::DIR_TEMP, &path))
            {
                return false;
            }

            path = path.Append(kAppUpdateDirName);
            create_dir = true;
            break;

        case lcpfw::DIR_UPDATE_REPORT:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Update"));
            create_dir = true;
            break;

        case lcpfw::FILE_RESOURCES_PACK:
            if (!PathService::Get(base::DIR_MODULE, &path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("app_100_percent.pak"));
            break;

        case lcpfw::DIR_APP_TEMP:
            if (!PathService::Get(base::DIR_TEMP, &path))
            {
                return false;
            }

            path = path.Append(kAppTempDirName);
            create_dir = true;
            break;

        case lcpfw::DIR_KV_CACHE:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("KV Cache"));
            create_dir = true;
            break;

        default:
            return false;
        }

        base::ThreadRestrictions::ScopedAllowIO allow_io;
        if (create_dir && !base::PathExists(path) &&
            !base::CreateDirectory(path))
        {
            return false;
        }

        *result = path;
        return true;
    }

    void RegisterPathProvider()
    {
        PathService::RegisterProvider(PathProvider, PATH_START, PATH_END);
    }
}
