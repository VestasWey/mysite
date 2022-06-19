#pragma once

#include "public/main/app_main_exports.h"

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

class AppModule;
enum class AppModuleType;

class APP_MAIN_LIB_EXPORT AppModuleManager
    : public base::RefCounted<AppModuleManager>
{
public:
    AppModuleManager();

    int Initialized();
    void Shutdown();

    base::WeakPtr<AppModule> GetModule(AppModuleType type);

private:
    ~AppModuleManager();

private:
    friend class base::RefCounted<AppModuleManager>;

    std::map<AppModuleType, std::unique_ptr<AppModule>> modules_;

    DISALLOW_COPY_AND_ASSIGN(AppModuleManager);
};
