#include "secret/app_secret.h"

#include "base/at_exit.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/process/launch.h"
#include "base/process/memory.h"
#include "base/strings/string_util.h"
#include "base/task/thread_pool/thread_pool_impl.h"
#include "base/time/time.h"

#include "common/app_context.h"
#include "common/app_logging.h"
#include "common/app_result_codes.h"
#include "common/app_constants.h"
#include "common/app_paths.h"
#include "common/profiles/profile.h"
#include "network/url_request/request_context.h"
#include "services/context_service.h"

AppSecret* g_secret_core = nullptr;

namespace {

const wchar_t kSecretPrefFileName[] = L"Secret Preference";

void RegisterLoginProfilePrefs(PrefRegistrySimple* registry)
{
    /*registry->RegisterStringPref(prefs::kLastLogin, "default");
    registry->RegisterIntegerPref(prefs::kLastLoginType, 0);
    registry->RegisterInt64Pref(prefs::kLastLoginUser, 0);

    base::ListValue *list_history = new base::ListValue();
    registry->RegisterListPref(prefs::kHistory, list_history);*/
}

void RegisterSecretProfilePrefs(PrefRegistrySimple* registry)
{
    RegisterLoginProfilePrefs(registry);
}

}   // namespace

class AppSecretImpl : public AppSecret
{
public:
    AppSecretImpl()
    {
        g_secret_core = this;
    }
    ~AppSecretImpl() override
    {
        g_secret_core = nullptr;
    }

    bool Initialize(
        base::FilePath profile_dir,
        scoped_refptr<base::SingleThreadTaskRunner> profile_task_runner,
        scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
        base::ThreadPoolInstance* thread_pool_instance) override
    {
        int argc = 0;
        char **argv = nullptr;
        base::CommandLine::Init(argc, argv);

        exit_manager_ = std::make_unique<base::AtExitManager>();

        base::EnableTerminationOnHeapCorruption();
        base::EnableTerminationOnOutOfMemory();
    #ifdef OS_WIN
        base::Time::EnableHighResolutionTimer(true);
        base::Time::ActivateHighResolutionTimer(true);
    #endif

        //base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

        lcpfw::RegisterPathProvider();
        
    #if defined(NDEBUG)
        bool enable_debug_logging = false;
        if (base::CommandLine::ForCurrentProcess()->HasSwitch(lcpfw::kSwitchDebugConsole)) {
            enable_debug_logging = true;
        }
    #else   // NDEBUG
        bool enable_debug_logging = true;
    #endif  // NDEBUG
        lcpfw::InitAppLogging(enable_debug_logging);

        AppContext::Current()->Init();

        // set ThreadTaskRunnerHandle for this module, no need to create new one.
        main_task_runner_ = main_task_runner;
        main_task_runner_handle_ = std::make_unique<base::ThreadTaskRunnerHandle>(main_task_runner);

        // set ThreadPoolInstance for this module, no need to create new one.
        base::internal::ThreadPoolImpl* thread_pool = static_cast<base::internal::ThreadPoolImpl*>(thread_pool_instance);
        base::ThreadPoolInstance::Set(std::unique_ptr<base::ThreadPoolInstance>(thread_pool));

        profile_ = Profile::CreateProfile(
            profile_dir.Append(kSecretPrefFileName),
            nullptr,
            RegisterSecretProfilePrefs,
            profile_task_runner);

        url_request_context_ = std::make_unique<UrlRequestContext>();

        context_service_ = ContextService::Create(url_request_context_.get());

        return true;
    }

    void Uninitialize() override
    {
        DCHECK(thread_checker_.CalledOnValidThread());

        // services must be released before UrlRequestContext release
        context_service_ = nullptr;

        url_request_context_ = nullptr;

        if (profile_)
        {
            profile_->GetPrefs()->CommitPendingWrite();
        }
        profile_ = nullptr;

        main_task_runner_ = nullptr;
    }

    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner() override
    {
        return main_task_runner_;
    }

    ContextService* context_service() override
    {
        return context_service_.get();
    }

    Profile* profile() override
    {
        DCHECK(profile_);
        return profile_.get();
    }

    PrefService* local_state() override
    {
        DCHECK(profile_);
        return profile_->GetPrefs();
    }

private:
    std::unique_ptr<base::AtExitManager> exit_manager_;
    std::unique_ptr<UrlRequestContext> url_request_context_;
    base::ThreadChecker thread_checker_;
    std::unique_ptr<Profile> profile_;

    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
    std::unique_ptr<base::ThreadTaskRunnerHandle> main_task_runner_handle_;

    std::unique_ptr<ContextService> context_service_;
};


AppSecret* GetAppSecret()
{
    static std::once_flag of;
    std::call_once(of,
        []() {
            if (!g_secret_core)
            {
                g_secret_core = new AppSecretImpl();
            }
        }
    );

    return g_secret_core;
}
