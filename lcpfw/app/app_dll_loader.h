#pragma once

#include "base/files/file_path.h"
#include "base/strings/string16.h"
#include "base/scoped_native_library.h"

using AppModuleEntry = int (*)();

class AppDllLoader
{
public:
    AppDllLoader(const base::FilePath& relative_module_path);

    virtual ~AppDllLoader();

    int Launch();
    base::FilePath module_path() const { return module_path_; }

protected:
    virtual void OnBeforeLaunch();
    virtual int DoLaunch(AppModuleEntry entry_point);
    virtual int OnBeforeExit(int return_code);

    static std::unique_ptr<base::ScopedNativeLibrary> LoadModule(const base::FilePath& module_path);

private:
    base::FilePath module_path_;
    std::unique_ptr<base::ScopedNativeLibrary> loadable_module_;
};
