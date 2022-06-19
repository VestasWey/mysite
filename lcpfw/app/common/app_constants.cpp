#include "common/app_constants.h"

namespace lcpfw {

    const char kSwitchProcessType[] = "app-type";
    const char kSwitchLaunchUIExamples[] = "ui-examples";
    const char kSwitchDebugConsole[] = "debug-console";
    const char kSwitchFullMinidump[] = "full-minidump";
    const char kSwitchParentPID[] = "parent-pid";
    const char kSwitchCrashRestart[] = "crash-restart";

    const char kAppWatcher[] = "app-watcher";

    const base::FilePath::CharType kAppFullName[] = FILE_PATH_LITERAL("lcpfw");
    const base::FilePath::CharType kUserDataDirname[] = FILE_PATH_LITERAL("User Data");
    const base::FilePath::CharType kGlobalProfileDirName[] = FILE_PATH_LITERAL("Global");
    const base::FilePath::CharType kPreferencesFilename[] = FILE_PATH_LITERAL("Preferences");
    const base::FilePath::CharType kAppLogFileName[] = FILE_PATH_LITERAL("app.log");
    const base::FilePath::CharType kAppUpdateDirName[] = FILE_PATH_LITERAL("lcpfw");
    const base::FilePath::CharType kAppTempDirName[] = FILE_PATH_LITERAL("lcpfw");

#ifdef OS_WIN
    const base::FilePath::CharType kAppMainDll[] = FILE_PATH_LITERAL("app_main.dll");
    const base::FilePath::CharType kAppResourcesDll[] = FILE_PATH_LITERAL("app_main.dll");
    const base::FilePath::CharType kAppSecretDll[] = FILE_PATH_LITERAL("app_secret.dll");
    const base::FilePath::CharType kAppWatcherDll[] = FILE_PATH_LITERAL("app_watcher.dll");
#else
    const base::FilePath::CharType kAppMainDll[] = FILE_PATH_LITERAL("libapp_main.dylib");
    const base::FilePath::CharType kAppResourcesDll[] = FILE_PATH_LITERAL("libapp_main.dylib");
    const base::FilePath::CharType kAppSecretDll[] = FILE_PATH_LITERAL("libapp_secret.dylib");
    const base::FilePath::CharType kAppWatcherDll[] = FILE_PATH_LITERAL("libapp_watcher.dylib");
#endif

    const int64_t kMaxFileSizeAllowedToUpload = 5 * 1024 * 1024;

    const char kLogBoundary[] = "----------app.log.boundary";

    // crash 
#if OS_WIN
    extern const wchar_t kExceptionHandlerPipeName[] = L"\\\\.\\pipe\\lcpfw\\ExceptionHandlerPipe";
    extern const wchar_t kExceptionHandlerReadyEventName[] = L"ExceptionHandlerReady";
#endif
    const char kDummyCrashID[] = "unknown";
    const int64_t kMaxCrashLogFileSize = 5 * 1024 * 1024;

}
