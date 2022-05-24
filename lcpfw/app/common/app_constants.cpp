#include "common/app_constants.h"

namespace lcpfw {

    const base::FilePath::CharType kAppFullName[] = FILE_PATH_LITERAL("lcpfw");
    const base::FilePath::CharType kUserDataDirname[] = FILE_PATH_LITERAL("User Data");
    const base::FilePath::CharType kGlobalProfileDirName[] = FILE_PATH_LITERAL("Global");
    const base::FilePath::CharType kPreferencesFilename[] = FILE_PATH_LITERAL("Preferences");

    const char kSwitchProcessType[] = "type";
    const char kSwitchLaunchUIExamples[] = "ui-examples";
    const char kSwitchDebugConsole[] = "debug-console";
    const base::FilePath::CharType kAppLogFileName[] = FILE_PATH_LITERAL("app.log");
    const base::FilePath::CharType kAppUpdateDirName[] = FILE_PATH_LITERAL("lcpfw");
    const base::FilePath::CharType kAppTempDirName[] = FILE_PATH_LITERAL("lcpfw");

#ifdef OS_WIN
    const base::FilePath::CharType kAppMainDll[] = FILE_PATH_LITERAL("app_main.dll");
    const base::FilePath::CharType kAppResourcesDll[] = FILE_PATH_LITERAL("app_main.dll");
    const base::FilePath::CharType kAppSecretDll[] = FILE_PATH_LITERAL("app_secret.dll");
#else
    const char kAppMainDll[] = "libapp_main.dylib";
    const char kAppResourcesDll[] = "libapp_main.dylib";
    const char kAppSecretDll[] = "libapp_secret.dylib";
#endif

    const int64_t kMaxFileSizeAllowedToUpload = 5 * 1024 * 1024;

    const char kLogBoundary[] = "----------app.log.boundary";
}
