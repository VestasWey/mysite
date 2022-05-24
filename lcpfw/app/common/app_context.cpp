#include "app_context.h"

#include <memory>

#include "base/files/file_util.h"
#include "base/file_version_info.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/version.h"

//#include "config/app_config.h"

#ifdef OS_WIN
#include <Windows.h>
#include "base/win/scoped_handle.h"
#endif

#ifdef __APPLE__
#include <sys/syslimits.h>
#include <mach-o/dyld.h>
#endif

#include "base/strings/utf_string_conversions.h"


namespace {

#ifdef _MSC_VER
    static const size_t kMaxPathBufSize = MAX_PATH + 1;
#else
    static const size_t kMaxPathBufSize = PATH_MAX + 1;
#endif

base::FilePath GetModuleExecutablePath()
{
    base::FilePath::CharType exe_path[kMaxPathBufSize] = {0};

#ifdef _MSC_VER
    GetModuleFileNameW(nullptr, exe_path, kMaxPathBufSize);
#else
    uint32_t len = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &len) != 0) {
        exe_path[0] = '\0'; // buffer too small (!)
        NOTREACHED();
    }
#endif

    base::FilePath epath(exe_path); 
    base::FilePath abs_path = base::MakeAbsoluteFilePath(epath);
    if (!abs_path.empty())
    {
        return abs_path;
    }

    return epath;
}


std::string GetFileVersion(const base::FilePath& file_path)
{
    /*std::unique_ptr<FileVersionInfo> version_info(
        FileVersionInfo::CreateFileVersionInfo(file_path));
    if (version_info) {
        std::string version_string(base::UTF16ToUTF8(version_info->file_version()));
        if (base::Version(version_string).IsValid())
        {
            return version_string;
        }
    }

    return {};*/
    //return APP_VERSION;
    return "1.0.0";
}

ApplicationMode GuessApplicationMode(const base::FilePath&)
{
    return ApplicationMode::DefaultMode;
}

}   // namespace

// static
AppContext* AppContext::Current()
{
    static AppContext instance;
    return &instance;
}

AppContext::AppContext()
{}

AppContext::~AppContext()
{}

void AppContext::Init()
{
    DCHECK(!inited_) << "AppContext can't be initialized twice!";

    exe_path_ = GetModuleExecutablePath();
    exe_ver_ = GetFileVersion(exe_path_);
    exe_dir_ = exe_path_.DirName();

    main_dll_path_ = GuessMainDLLPath(exe_dir_, exe_ver_);
    main_dir_ = main_dll_path_.DirName();

    app_mode_ = GuessApplicationMode(main_dir_);

    inited_ = true;
}

const base::FilePath& AppContext::GetExecutablePath() const
{
    DCHECK(!exe_path_.empty());
    return exe_path_;
}

const base::FilePath& AppContext::GetExecutableDirectory() const
{
    DCHECK(!exe_dir_.empty());
    return exe_dir_;
}

const base::FilePath& AppContext::GetMainDirectory() const
{
    DCHECK(!main_dir_.empty());
    return main_dir_;
}

const base::FilePath& AppContext::GetMainDLLPath() const
{
    DCHECK(!main_dll_path_.empty());
    return main_dll_path_;
}

const std::string& AppContext::GetExecutableVersion() const
{
    DCHECK(!exe_ver_.empty());
    return exe_ver_;
}

unsigned short AppContext::GetExecutableBuildNumber() const
{
    base::Version ver(GetExecutableVersion());
    const auto& components = ver.components();
    if (components.empty()) {
        return 0;
    }

    return components.back() == 0 ? 9999 : (unsigned short)components.back();
}

ApplicationMode AppContext::GetApplicationMode() const
{
    return app_mode_;
}

bool AppContext::InApplicationMode(ApplicationMode mode) const
{
    return app_mode_ == mode;
}
