#include "main_dll_loader.h"

#include <memory>

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"

#include "common/app_context.h"

namespace {

    void AddMainDirectoryIntoPathEnv(const base::FilePath& main_directory)
    {
        std::unique_ptr<base::Environment> env(base::Environment::Create());
        std::string path_env;
        bool succeeded = env->GetVar("path", &path_env);
        if (!succeeded)
        {
            PLOG(WARNING) << "Failed to read path env";
            return;
        }

        std::string patched_path_env = main_directory.AsUTF8Unsafe();
        patched_path_env.append(";").append(path_env);
        succeeded = env->SetVar("path", patched_path_env);
        PLOG_IF(WARNING, !succeeded) << "Failed to update path env";
    }

} // namespace

MainDllLoader::MainDllLoader(const base::FilePath& relative_module_path)
    : AppDllLoader(relative_module_path)
{
}

MainDllLoader::~MainDllLoader()
{
}

void MainDllLoader::OnBeforeLaunch()
{
#ifdef OS_WIN
    // Make main directory as current directory to ensure all dependency dlls being loaded successfully.
    auto main_dir = AppContext::Current()->GetMainDirectory();
    DCHECK(!main_dir.empty());
    SetCurrentDirectoryW(main_dir.value().c_str());
#endif

    // In some as-yet unknown circumstances, SetCurrentDirectory() still failed to guarantee all
    // dependency dlls being loaded successfully, especially for those who are imported implicitly.
    // Add main directory into PATH environment variable as the last resort.
    AddMainDirectoryIntoPathEnv(main_dir);
}
