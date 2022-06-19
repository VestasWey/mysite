#include "app_dll_loader.h"

#include <memory>

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"

#include "common/app_context.h"

AppDllLoader::AppDllLoader(const base::FilePath& relative_module_path)
{
    auto dll_dir = AppContext::Current()->GetMainDirectory();
    module_path_ = dll_dir.Append(relative_module_path);
}

AppDllLoader::~AppDllLoader()
{}

std::unique_ptr<base::ScopedNativeLibrary> AppDllLoader::LoadModule(const base::FilePath& module_path)
{
    auto dll = std::make_unique<base::ScopedNativeLibrary>(module_path);
    if (!dll->is_valid()) {
        PLOG(ERROR) << "Failed to load module from " << module_path.AsUTF8Unsafe();
        return nullptr;
    }

    return dll;
}

int AppDllLoader::Launch()
{
    loadable_module_ = LoadModule(module_path_);
    if (!loadable_module_) {
        return -1;
    }

    AppModuleEntry entry_point = reinterpret_cast<AppModuleEntry>(loadable_module_->GetFunctionPointer("AppModuleEntry"));
    if (!entry_point)
    {
        return -1;
    }

    OnBeforeLaunch();

    int rc = DoLaunch(entry_point);

    return OnBeforeExit(rc);
}

void AppDllLoader::OnBeforeLaunch()
{}

int AppDllLoader::DoLaunch(AppModuleEntry entry_point)
{
    return entry_point();
}

int AppDllLoader::OnBeforeExit(int return_code)
{
    return return_code;
}

