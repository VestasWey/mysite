#pragma once

#include "base/files/file_path.h"

namespace lcpfw {

    extern const char kSwitchProcessType[];
    extern const char kSwitchLaunchUIExamples[];
    extern const char kSwitchDebugConsole[];
    extern const char kSwitchFullMinidump[];
    extern const char kSwitchParentPID[];
    extern const char kSwitchCrashRestart[];

    extern const char kAppWatcher[];

    extern const base::FilePath::CharType kAppFullName[];
    extern const base::FilePath::CharType kUserDataDirname[];
    extern const base::FilePath::CharType kGlobalProfileDirName[];
    extern const base::FilePath::CharType kPreferencesFilename[];
    extern const base::FilePath::CharType kAppLogFileName[];
    extern const base::FilePath::CharType kAppUpdateDirName[];
    extern const base::FilePath::CharType kAppTempDirName[];
    extern const base::FilePath::CharType kAppMainDll[];
    extern const base::FilePath::CharType kAppResourcesDll[];
    extern const base::FilePath::CharType kAppSecretDll[];
    extern const base::FilePath::CharType kAppWatcherDll[];

    extern const int64_t kMaxFileSizeAllowedToUpload;

    extern const char kLogBoundary[];

    // crash 
#if OS_WIN
    extern const wchar_t kExceptionHandlerPipeName[];
    extern const wchar_t kExceptionHandlerReadyEventName[];
#endif
    extern const char kDummyCrashID[];
    extern const int64_t kMaxCrashLogFileSize;

}
