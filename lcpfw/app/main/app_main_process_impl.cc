#include "main/app_main_process_impl.h"

#ifdef OS_WIN
#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#endif

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/scoped_native_library.h"
#include "base/debug/alias.h"
#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
//#include "components/keep_alive_registry/keep_alive_registry.h"
//#include "components/prefs/json_pref_store.h"
//#include "net/log/net_log.h"

#include "common/app_context.h"
#include "common/app_constants.h"
#include "common/app_pref_names.h"
#include "content/app_thread.h"
#include "profiles/main_profile.h"
#include "public/main/app_notification_types.h"
#include "secret/app_secret.h"


namespace
{
    AppMainProcessImpl* g_app_process = nullptr;

    void NotifyAppTerminating()
    {
        static bool notified = false;
        if (notified) {
            return;
        }

        notified = true;
    }

    void OnAppExiting()
    {
        static bool notified = false;
        if (notified) {
            return;
        }

        notified = true;

        //views::Widget::CloseAllSecondaryWidgets();
    }

    class UserProfileDelegate : public Profile::Delegate {
    public:
        void OnProfileCreated(Profile* profile, bool success, bool is_new_profile) override
        {
            // 检查这个回调传入的配置文件确实不是仅程序相关的公共配置文件
            DCHECK(!base::EndsWith(profile->GetPath().value(), lcpfw::kGlobalProfileDirName, base::CompareCase::SENSITIVE));
            DCHECK(success);

            // do sth
            //////////////////////////////////////////////////////////////////////////
        }
    };

}


AppMainProcess* GetAppMainProcess()
{
    DCHECK(g_app_process);
    return g_app_process;
}

// static
void AppMainProcessImpl::RegisterGlobalPrefs(PrefRegistrySimple* registry)
{
    bool drag_full_windows = true;
#ifdef OS_WIN
    BOOL win_drag_full_windows = TRUE;
    ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, NULL, &win_drag_full_windows, NULL);
    drag_full_windows = !!win_drag_full_windows;
#endif
    registry->RegisterBooleanPref(prefs::kDragFullWindows, drag_full_windows);
    registry->RegisterBooleanPref(prefs::kApplicationExitRememberChoice, false);
}

void AppMainProcessImpl::RegisterUserPrefs(PrefRegistrySimple* registry)
{
    registry->RegisterStringPref(prefs::kAvatarCacheTag, "");
    registry->RegisterStringPref(prefs::kLastLoginDate, "");
}

AppMainProcessImpl::AppMainProcessImpl(const base::FilePath& user_data_dir)
    : user_data_dir_(user_data_dir)
{
    g_app_process = this;
}

AppMainProcessImpl::~AppMainProcessImpl()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    //tracked_objects::ThreadData::EnsureCleanupWasCalled(4);

//#if !defined(OS_ANDROID)
//    KeepAliveRegistry::GetInstance()->RemoveObserver(this);
//#endif

    g_app_process = nullptr;
}

void AppMainProcessImpl::Init()
{
#if defined(OS_MAC)
    ui::InitIdleMonitor();
#endif

#if !defined(OS_ANDROID)
    //KeepAliveRegistry::GetInstance()->SetIsShuttingDown(false);
    //KeepAliveRegistry::GetInstance()->AddObserver(this);
#endif  // !defined(OS_ANDROID)
}

void AppMainProcessImpl::SetQuitClosure(base::OnceClosure quit_closure) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK(quit_closure);
    DCHECK(!quit_closure_);
    quit_closure_ = std::move(quit_closure);
}

#if defined(OS_MAC)
void AppMainProcessImpl::ClearQuitClosure() {
    quit_closure_.Reset();
}
#endif

bool AppMainProcessImpl::IsShuttingDown()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    return shutting_down_ || tearing_down_;
}

void AppMainProcessImpl::SetApplicationLocale(const std::string& locale)
{
    DCHECK(!locale.empty());
    locale_ = locale;
}

const std::string& AppMainProcessImpl::GetApplicationLocale()
{
    DCHECK(!locale_.empty());
    return locale_;
}

Profile* AppMainProcessImpl::global_profile()
{
    DCHECK(global_profile_.get());
    return global_profile_.get();
}

Profile *AppMainProcessImpl::profile()
{
    DCHECK(profile_.get());
    return profile_.get();
}

PrefService* AppMainProcessImpl::local_state()
{
    Profile* pf = profile();
    if (pf)
    {
        return pf->GetPrefs();
    }

    return nullptr;
}

PrefService* AppMainProcessImpl::global_state()
{
    Profile* pf = global_profile();
    if (pf)
    {
        return pf->GetPrefs();
    }

    return nullptr;
}

// internal
void AppMainProcessImpl::PreCreateThreads()
{
    InitGlobalProfile();
}

bool AppMainProcessImpl::PreMainMessageLoopRun()
{
    if (!LoadSecretModule())
    {
        return false;
    }

    // 临近开启消息循环，确保UI所需的相关模块在这之前都初始化好了
    // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    return true;
}

void AppMainProcessImpl::InitGlobalProfile()
{
    auto global_profile_dir = user_data_dir_.Append(lcpfw::kGlobalProfileDirName);
    if (!base::DirectoryExists(global_profile_dir)) {
        base::CreateDirectory(global_profile_dir);
    }

    if (!profile_task_runner_)
    {
        profile_task_runner_ =
            base::CreateSingleThreadTaskRunner(
                { base::ThreadPool(), base::MayBlock(),
                 base::TaskPriority::USER_VISIBLE,
                 base::TaskShutdownBehavior::BLOCK_SHUTDOWN });
    }

    global_profile_ = MainProfile::CreateGlobalProfile(
        global_profile_dir,
        nullptr,
        profile_task_runner_);
}

void AppMainProcessImpl::InitLocalProfile()
{
    // TODO: auto user_profile_dir = user_data_dir_.Append(user account ID);
    NOTREACHED();
    auto user_profile_dir = user_data_dir_.Append(lcpfw::kGlobalProfileDirName);
    if (!base::DirectoryExists(user_profile_dir))
    {
        base::CreateDirectory(user_profile_dir);
    }

    if (!profile_task_runner_)
    {
        profile_task_runner_ =
            base::CreateSingleThreadTaskRunner(
                { base::ThreadPool(), base::MayBlock(),
                 base::TaskPriority::USER_VISIBLE,
                 base::TaskShutdownBehavior::BLOCK_SHUTDOWN });
    }

    UserProfileDelegate delegate;

    profile_ = MainProfile::CreateProfile(
        user_profile_dir,
        &delegate,
        profile_task_runner_);
}

bool AppMainProcessImpl::LoadSecretModule()
{
    auto dll_path = AppContext::Current()->GetMainDirectory();
    dll_path = dll_path.Append(lcpfw::kAppSecretDll);
    secret_dll_.reset(new base::ScopedNativeLibrary(dll_path));
    if (!secret_dll_->is_valid())
    {
        auto err = secret_dll_->GetError();
        LOG(ERROR) << "Failed to load secret library, err = " << err->ToString();

        secret_dll_ = nullptr;
        return false;
    }

    SecretModuleEntry entry_point = reinterpret_cast<SecretModuleEntry>(secret_dll_->GetFunctionPointer("AppSecretEntry"));
    if (!entry_point)
    {
        secret_dll_ = nullptr;
        return false;
    }

    secret_module_ = entry_point();
    if (!secret_module_)
    {
        secret_dll_ = nullptr;
        return false;
    }

    auto global_profile_dir = user_data_dir_.Append(lcpfw::kGlobalProfileDirName);
    if (!secret_module_->Initialize(global_profile_dir, 
        profile_task_runner_, 
        base::ThreadTaskRunnerHandle::Get(),
        base::ThreadPoolInstance::Get()))
    {
        secret_dll_ = nullptr;
        return false;
    }

    return true;
}

//void AppMainProcessImpl::OnKeepAliveStateChanged(bool is_keeping_alive) {
//    if (is_keeping_alive)
//        Pin();
//    else
//        Unpin();
//}
//
//void AppMainProcessImpl::OnKeepAliveRestartStateChanged(bool can_restart)
//{
//}

void AppMainProcessImpl::Pin()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK(!IsShuttingDown());

    //if (IsShuttingDown()) {
    //    base::debug::StackTrace callstack = release_last_reference_callstack_;
    //    base::debug::Alias(&callstack);
    //    CHECK(false);
    //}
}

void AppMainProcessImpl::Unpin()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

#if !defined(OS_ANDROID)
    // The quit closure is set by ChromeBrowserMainParts to transfer ownership of
    // the browser's lifetime to the BrowserProcess. Any KeepAlives registered and
    // unregistered prior to setting the quit closure are ignored. Only once the
    // quit closure is set should unpinning start process shutdown.
    if (!quit_closure_)
        return;
#endif

    DCHECK(!shutting_down_);
    shutting_down_ = true;

#if !defined(OS_ANDROID)
    //KeepAliveRegistry::GetInstance()->SetIsShuttingDown();
#endif  // !defined(OS_ANDROID)

    DCHECK(base::RunLoop::IsRunningOnCurrentThread());

//#if defined(OS_MAC)
//    base::PostTask(
//        FROM_HERE,
//        base::BindOnce(ChromeBrowserMainPartsMac::DidEndMainMessageLoop));
//#endif

#if !defined(OS_ANDROID)
    std::move(quit_closure_).Run();
    LOG(INFO) << "Main MessageLoop Quit";

    //chrome::ShutdownIfNeeded();
#endif
}

void AppMainProcessImpl::StartTearDown()
{
    tearing_down_ = true;
    DCHECK(IsShuttingDown());

    if (global_profile_)
    {
        global_profile_->GetPrefs()->CommitPendingWrite();
    }

    if (profile_)
    {
        profile_->GetPrefs()->CommitPendingWrite();
    }

    if (secret_module_)
    {
        secret_module_->Uninitialize();
        secret_module_ = nullptr;
    }
}

void AppMainProcessImpl::PostDestroyThreads()
{
    profile_task_runner_ = nullptr;
}

void AppMainProcessImpl::EndSession()
{

}

void AppMainProcessImpl::FlushLocalStateAndReply(base::OnceClosure reply)
{

}
