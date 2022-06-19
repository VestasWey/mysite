#pragma once

#include "base/files/file_path.h"
#include "base/strings/string16.h"
#include "base/scoped_native_library.h"

class MainDllLoader
{
public:
    MainDllLoader();

    virtual ~MainDllLoader();

    int Launch();

protected:
    virtual base::FilePath GetRegistryPath() = 0;

    virtual void OnBeforeLaunch(const base::FilePath& main_dir);

    virtual int OnBeforeExit(int return_code, const base::FilePath& main_dir);

    base::ScopedNativeLibrary* Load(base::FilePath* module_dir);

private:
    std::unique_ptr<base::ScopedNativeLibrary> dll_;
};

std::unique_ptr<MainDllLoader> MakeMainDllLoader();
