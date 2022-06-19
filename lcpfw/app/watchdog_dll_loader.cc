#include "main_dll_loader.h"

#include <memory>

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"

#include "common/app_context.h"

namespace {

using DLL_MAIN = int (*)();

base::ScopedNativeLibrary* LoadAppMainDLL(const base::FilePath& main_dll_path)
{
    std::unique_ptr<base::ScopedNativeLibrary> dll(new base::ScopedNativeLibrary(main_dll_path));
    if (dll->is_valid())
    {
        return dll.release();
    }
    else
    {
        auto err = dll->GetError();
        LOG(ERROR) << "Failed to load main library, err = " << err->ToString();
    }
    return nullptr;
}

void AddMainDirectoryIntoPathEnv(const base::FilePath& main_directory)
{
    std::unique_ptr<base::Environment> env(base::Environment::Create());
    std::string path_env;
    bool succeeded = env->GetVar("path", &path_env);
    if (!succeeded) {
        PLOG(WARNING) << "Failed to read path env";
        return;
    }

    std::string patched_path_env = main_directory.AsUTF8Unsafe();
    patched_path_env.append(";").append(path_env);
    succeeded = env->SetVar("path", patched_path_env);
    PLOG_IF(WARNING, !succeeded) << "Failed to update path env";
}

} // namespace

MainDllLoader::MainDllLoader()
{}

MainDllLoader::~MainDllLoader()
{}

base::ScopedNativeLibrary* MainDllLoader::Load(base::FilePath* module_dir)
{
    auto dll_path = AppContext::Current()->GetMainDLLPath();
    *module_dir = dll_path.DirName();
    std::unique_ptr<base::ScopedNativeLibrary> dll(LoadAppMainDLL(dll_path));

    if (!dll) {
        PLOG(ERROR) << "Failed to load app main dll from " << dll_path.AsUTF8Unsafe();
        return nullptr;
    }

    return dll.release();
}

int MainDllLoader::Launch()
{
    base::FilePath main_dir;
    dll_.reset(Load(&main_dir));
    if (!dll_) {
        return -1;
    }

#ifdef OS_WIN
    // Make main directory as current directory to ensure all dependency dlls being loaded successfully.
    DCHECK(!main_dir.empty());
    SetCurrentDirectoryW(main_dir.value().c_str());
#endif

    // In some as-yet unknown circumstances, SetCurrentDirectory() still failed to guarantee all
    // dependency dlls being loaded successfully, especially for those who are imported implicitly.
    // Add main directory into PATH environment variable as the last resort.
    AddMainDirectoryIntoPathEnv(main_dir);

    OnBeforeLaunch(main_dir);

    DLL_MAIN entry_point = reinterpret_cast<DLL_MAIN>(dll_->GetFunctionPointer("AppMainEntry"));
    if (!entry_point) {
        return -1;
    }

    int rc = entry_point();

    return OnBeforeExit(rc, main_dir);
}

void MainDllLoader::OnBeforeLaunch(const base::FilePath& main_dir)
{}

int MainDllLoader::OnBeforeExit(int return_code, const base::FilePath& main_dir)
{
    return return_code;
}


// AppDllLoader
class AppDllLoader
    : public MainDllLoader {
public:
    base::FilePath GetRegistryPath() override
    {
        return base::FilePath();
    }

    void OnBeforeLaunch(const base::FilePath& main_dir) override
    {}

    int OnBeforeExit(int return_code, const base::FilePath& main_dir) override
    {
        return 0;
    }
};

std::unique_ptr<MainDllLoader> MakeMainDllLoader()
{
    return std::make_unique<AppDllLoader>();
}

