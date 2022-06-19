#pragma once

#include "app_dll_loader.h"

class MainDllLoader : public AppDllLoader
{
public:
    MainDllLoader(const base::FilePath& relative_module_path);
    virtual ~MainDllLoader();

protected:
    void OnBeforeLaunch() override;
};
