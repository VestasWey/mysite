#include "main/app_main_parts_impl.h"

#ifdef OS_WIN
//#include <atlcomcli.h>
//#include <dxgi.h>

#include "base/win/windows_version.h"
//#include "ui/base/l10n/l10n_util_win.h"
//#include "ui/base/resource/resource_bundle_win.h"
//#include "base/trace_event/trace_event_etw_export_win.h"

//#include "main/win/app_select_file_dialog_factory.h"

#endif

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/system/sys_info.h"
#include "base/task/current_thread.h"
//#include "components/device_event_log/device_event_log.h"
//#include "components/prefs/pref_service.h"
//
//#include "ui/base/l10n/l10n_util.h"
//#include "ui/base/resource/resource_bundle.h"
//#include "ui/views/focus/accelerator_handler.h"

//#include "app/app/ui/examples/examples_runner.h"
#include "common/app_context.h"
#include "common/app_paths.h"
#include "common/app_pref_names.h"
#include "common/app_result_codes.h"
#include "main/app_main_extra_parts_views.h"



namespace
{
    std::unique_ptr<base::RunLoop> g_run_loop;

    void LogSystemInformation()
    {
        const size_t kPresumedSize = 256;
        const char kFenceBar[] = "------------SYS INFO------------";

        std::string diagnose_data;
        diagnose_data.reserve(kPresumedSize);
        diagnose_data.append("\n").append(kFenceBar).append("\n");

        const char item_app_ver[] = "Application Version: ";
        auto app_version = AppContext::Current()->GetExecutableVersion();
        diagnose_data.append(item_app_ver).append(app_version).append("\n");

        const char item_win_ver[] = "Windows Version: ";
        auto os_version = base::SysInfo::OperatingSystemVersion();
        auto os_arch = base::SysInfo::OperatingSystemArchitecture();
        diagnose_data.append(item_win_ver).append(os_version).append(" ").append(os_arch).append("\n");

        const char item_exe_path[] = "EXE Path: ";
        base::FilePath exe_path;
        base::PathService::Get(base::FILE_EXE, &exe_path);
        diagnose_data.append(item_exe_path).append(exe_path.AsUTF8Unsafe()).append("\n");

        diagnose_data.append(kFenceBar);

        LOG(INFO) << diagnose_data;
    }
}

std::unique_ptr<AppMainParts> CreateAppMainParts(const MainFunctionParams& main_function_params)
{
    std::unique_ptr<AppMainPartsImpl> main_parts = std::make_unique<AppMainPartsImpl>(main_function_params);

    // Add some ExtraParts
    main_parts->AddParts(std::make_unique<AppMainExtraPartsViews>());

    return main_parts;
}

AppMainPartsImpl::AppMainPartsImpl(const MainFunctionParams& main_function_params)
    : parameters_(main_function_params),
      parsed_command_line_(main_function_params.command_line),
      result_code_(lcpfw::ResultCodeNormalExit)
{
}

AppMainPartsImpl::~AppMainPartsImpl()
{
    // Delete parts in the reverse of the order they were added.
    while (!app_extra_parts_.empty())
    {
        app_extra_parts_.pop_back();
    }
}

// AppMainParts implementation ------------------------------------

int AppMainPartsImpl::PreEarlyInitialization()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreEarlyInitialization();
    }

    base::PathService::Get(lcpfw::DIR_USER_DATA, &user_data_dir_);
    DCHECK(!user_data_dir_.empty());

    // Create BrowserProcess in PreEarlyInitialization() so that we can load
    // field trials (and all it depends upon).
    app_process_.reset(new AppMainProcessImpl(user_data_dir_));

    OnLocalStateLoaded();

    return 0;
}

void AppMainPartsImpl::PostEarlyInitialization()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostEarlyInitialization();
    }
}

void AppMainPartsImpl::ToolkitInitialized()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->ToolkitInitialized();
    }
}

void AppMainPartsImpl::PreMainMessageLoopStart()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreMainMessageLoopStart();
    }
}

void AppMainPartsImpl::PostMainMessageLoopStart()
{
    //ThreadProfiler::SetMainThreadTaskRunner(base::ThreadTaskRunnerHandle::Get());

    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostMainMessageLoopStart();
    }
}

int AppMainPartsImpl::PreCreateThreads()
{
    result_code_ = PreCreateThreadsImpl();

    if (result_code_ == lcpfw::ResultCodeNormalExit)
    {
        // These members must be initialized before exiting this function normally.
        // DCHECK(browser_creator_.get());

        for (size_t i = 0; i < app_extra_parts_.size(); ++i)
        {
            app_extra_parts_[i]->PreCreateThreads();
        }
    }

    return result_code_;
}

int AppMainPartsImpl::PreCreateThreadsImpl()
{
    run_message_loop_ = false;

    if (app_process_->GetApplicationLocale().empty()) {
        //ShowMissingLocaleMessageBox();
        return lcpfw::ResultCodeMissingData;
    }

    //process_singleton_ = std::make_unique<AppProcessSingleton>();

    // Cache first run state early.
    //first_run::IsChromeFirstRun();

    app_process_->Init();

    // Create the RunLoop for MainMessageLoopRun() to use, and pass a copy of
    // its QuitClosure to the BrowserProcessImpl to call when it is time to exit.
    DCHECK(!g_run_loop);
    //g_run_loop.reset(new base::RunLoop(base::RunLoop::Type::kNestableTasksAllowed));

    // These members must be initialized before returning from this function.
    // Android doesn't use StartupBrowserCreator.
    //app_creator_.reset(new StartupAppCreator);

#if defined(OS_WIN)
    // This is needed to enable ETW exporting. This is only relevant for the
    // browser process, as other processes enable it separately.
    //base::trace_event::TraceEventETWExport::EnableETWExport();
#endif  // OS_WIN

#if defined(OS_MAC)
    // Get the Keychain API to register for distributed notifications on the main
    // thread, which has a proper CFRunloop, instead of later on the I/O thread,
    // which doesn't. This ensures those notifications will get delivered
    // properly. See issue 37766.
    // (Note that the callback mask here is empty. I don't want to register for
    // any callbacks, I just want to initialize the mechanism.)
    SecKeychainAddCallback(&KeychainCallback, 0, nullptr);
#endif  // defined(OS_MAC)

    // needs ui::ResourceBundle::InitSharedInstance to be called before this.
    app_process_->PreCreateThreads();

    return lcpfw::ResultCodeNormalExit;
}

void AppMainPartsImpl::PostCreateThreads()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostCreateThreads();
    }
}

void AppMainPartsImpl::PreMainMessageLoopRun()
{
    result_code_ = PreMainMessageLoopRunImpl();

    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreMainMessageLoopRun();
    }
}

// PreMainMessageLoopRun calls these extra stages in the following order:
//  PreMainMessageLoopRunImpl()
//   ... initial setup, including browser_process_ setup.
//   PreProfileInit()
//   ... additional setup, including CreateProfile()
//   PostProfileInit()
//   ... additional setup
//   PreBrowserStart()
//   ... browser_creator_->Start (OR parameters().ui_task->Run())
//   PostBrowserStart()

void AppMainPartsImpl::PreProfileInit()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreProfileInit();
    }
}

void AppMainPartsImpl::PostProfileInit()
{
    // g_browser_process->CreateDevToolsProtocolHandler();
    // if (parsed_command_line().HasSwitch(::switches::kAutoOpenDevToolsForTabs))
    //     g_browser_process->CreateDevToolsAutoOpener();

    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostProfileInit();
    }
}

void AppMainPartsImpl::PreAppStart()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PreAppStart();
    }
}

void AppMainPartsImpl::AppStart()
{
    //int result_code;
    //if (app_creator_->Start(parsed_command_line(), user_data_dir_, &result_code))
    //{
    //    // Transfer ownership of the browser's lifetime to the BrowserProcess.
    //    app_process_->SetQuitClosure(g_run_loop->QuitWhenIdleClosure());
    //    DCHECK(!run_message_loop_);
    //    run_message_loop_ = true;
    //}
    //else
    //{
    //    run_message_loop_ = false;
    //}
    //app_creator_.reset();
}

void AppMainPartsImpl::PostAppStart()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i)
    {
        app_extra_parts_[i]->PostAppStart();
    }

    // Allow ProcessSingleton to process messages.
    //process_singleton_->Unlock();

}

int AppMainPartsImpl::PreMainMessageLoopRunImpl()
{
    /*if (!process_singleton_->Install())
    {
        return lcpfw::ResultCodeErrorOccurred;
    }*/

    if (!app_process_->PreMainMessageLoopRun()) {
        return lcpfw::ResultCodeErrorOccurred;
    }

    LogSystemInformation();

    // When to install crash client is a little bit tricky, we must ensure that:
    // (1) installing is not prior to mutiple application instances check.
    // (2) installing is prior to main work stuff.

    /*bool wait_result = lcpfw::WaitForCrashServerReady();
    if (!wait_result) {
        LOG(WARNING) << "Failed to bind with crash server; Turn into in-process mode";
    }

    crash_client_ = std::make_unique<lcpfw::CrashHandlerClient>();
    crash_client_->Install();*/

#if defined(OS_WIN)
    //ui::SelectFileDialog::SetFactory(new AppSelectFileDialogFactory());
#endif  // defined(OS_WIN)

    // Desktop construction occurs here, (required before profile creation).
    PreProfileInit();

    // 进行身份验证，为每个验证通过的用户创建其专属的配置文件
    //////////////////////////////////////////////////////////////////////////

    PostProfileInit();

    // Configure modules that need access to resources.
    //net::NetModule::SetResourceProvider(ChromeNetResourceProvider);
    //media::SetLocalizedStringProvider(ChromeMediaLocalizedStringProvider);

    PreAppStart();

    AppStart();

    PostAppStart();

    app_started_ = true;

    return lcpfw::ResultCodeNormalExit;
}

bool AppMainPartsImpl::MainMessageLoopRun(int* result_code)
{
    *result_code = result_code_;
    if (!run_message_loop_)
    {
        return false;  // Don't run the default message loop.
    }

    DCHECK(base::CurrentUIThread::IsSet());
    g_run_loop->Run();

    return true;
}

void AppMainPartsImpl::PostMainMessageLoopRun()
{
    for (size_t i = 0; i < app_extra_parts_.size(); ++i) {
        app_extra_parts_[i]->PostMainMessageLoopRun();
    }

    // if (notify_result_ == ProcessSingleton::PROCESS_NONE)
    //     process_singleton_->Cleanup();

    app_process_->StartTearDown();
}

void AppMainPartsImpl::PostDestroyThreads()
{
    app_process_->PostDestroyThreads();
    //app_process_.reset(nullptr);
    // browser_shutdown takes care of deleting browser_process, so we need to
    // release it.
    ignore_result(app_process_.release());

    //process_singleton_.reset();

    //device_event_log::Shutdown();
}

void AppMainPartsImpl::OnLocalStateLoaded()
{
    app_process_->SetApplicationLocale(prefs::kLocaleZhCN);
    /*auto locale_loaded = ui::ResourceBundle::InitSharedInstanceWithLocale(
        prefs::kLocaleZhCN, nullptr, ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);
    DCHECK_EQ(locale_loaded, prefs::kLocaleZhCN);
    std::vector<ui::ScaleFactor> supported_scale_factors;
    supported_scale_factors.push_back(ui::SCALE_FACTOR_100P);
#if defined(OS_APPLE) || defined(OS_WIN)
    supported_scale_factors.push_back(ui::SCALE_FACTOR_150P);
    supported_scale_factors.push_back(ui::SCALE_FACTOR_200P);
    supported_scale_factors.push_back(ui::SCALE_FACTOR_250P);
    supported_scale_factors.push_back(ui::SCALE_FACTOR_300P);
#endif
    ui::SetSupportedScaleFactors(supported_scale_factors);

    app_process_->SetApplicationLocale(locale_loaded);

#ifdef _WIN32
    ui::SetResourcesDataDLL(GetModuleHandleW(lcpfw::kAppResourcesDll));
#endif

    base::FilePath resources_pack_dir;
    if (base::PathService::Get(lcpfw::DIR_RESOURCES, &resources_pack_dir))
    {
        static const std::vector<std::pair<ui::ScaleFactor, base::FilePath>> paks {
            { ui::SCALE_FACTOR_100P, base::FilePath(L"app_100_percent.pak") },
            { ui::SCALE_FACTOR_150P, base::FilePath(L"app_150_percent.pak") },
            { ui::SCALE_FACTOR_200P, base::FilePath(L"app_200_percent.pak") },
            { ui::SCALE_FACTOR_250P, base::FilePath(L"app_250_percent.pak") },
            { ui::SCALE_FACTOR_300P, base::FilePath(L"app_300_percent.pak") },
        };
        for (auto& item : paks)
        {
            auto pak = resources_pack_dir.Append(item.second);
            if (base::PathExists(pak))
            {
                ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
                    pak, item.first);
            }
        }
    }
    else
    {
        NOTREACHED();
    }*/
}

void AppMainPartsImpl::AddParts(std::unique_ptr<AppMainExtraParts> parts)
{
    app_extra_parts_.push_back(std::move(parts));
}
