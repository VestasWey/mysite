#pragma once
#include "base/files/file_path.h"
#include "base/memory/scoped_refptr.h"
#include "base/single_thread_task_runner.h"
#include "base/task/thread_pool/thread_pool_instance.h"

class Profile;
class PrefService;
class ContextService;
class AppSecret;

using SecretModuleEntry = AppSecret*(*)();

class AppSecret : public base::RefCountedThreadSafe<AppSecret>
{
public:
    virtual bool Initialize(base::FilePath profile_dir,
        scoped_refptr<base::SingleThreadTaskRunner> profile_task_runner,
        scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
        base::ThreadPoolInstance* thread_pool_instance) = 0;

    virtual void Uninitialize() = 0;

    virtual scoped_refptr<base::SingleThreadTaskRunner> main_task_runner() = 0;

    virtual ContextService* context_service() = 0;

    virtual Profile* profile() = 0;
    virtual PrefService* local_state() = 0;

protected:
    friend class base::RefCountedThreadSafe<AppSecret>;
    virtual ~AppSecret() = default;
};

AppSecret* GetAppSecret();

