
#ifdef OS_WIN
#include <Windows.h>
#endif

#include "base/at_exit.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/i18n/icu_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/process/launch.h"
#include "base/process/memory.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"

//#include "components/viz/common/features.h"

//#include "mojo/core/embedder/embedder.h"
//#include "ui/base/ui_base_paths.h"
//#include "ui/base/ime/init/input_method_initializer.h"
//#include "ui/base/resource/resource_bundle.h"
//#include "ui/compositor/compositor_switches.h"
//#include "ui/compositor/app_in_process_context_factory.h"
//#include "ui/compositor/app_context_factories.h"
//#include "ui/display/screen.h"
//#include "ui/gfx/font_util.h"
//#include "ui/gl/gl_switches.h"
//#include "ui/gl/init/gl_factory.h"
//#include "ui/views/buildflags.h"

//#if defined(USE_AURA)
//#include "ui/aura/env.h"
//#include "ui/wm/core/wm_state.h"
//#endif

//#if BUILDFLAG(ENABLE_DESKTOP_AURA)
//#include "ui/views/widget/desktop_aura/desktop_screen.h"
//#endif

#if defined(OS_WIN)
//#include "ui/base/win/scoped_ole_initializer.h"
//#include "ui/views/examples/examples_skia_gold_pixel_diff.h"
#endif

//#include "components/viz/app_gpu_service_holder.h"
//#include "ui/gl/app_gl_surface_support.h"

#include "common/app_context.h"
#include "common/app_logging.h"
#include "common/app_result_codes.h"
#include "common/app_constants.h"
#include "common/app_paths.h"
#include "content/app_runner.h"
#include "main/app_main_parts_impl.h"


namespace {

void RestartApp()
{
    base::LaunchOptions launch_options;

    base::CommandLine command_line(base::CommandLine::ForCurrentProcess()->GetProgram());
    //command_line.AppendSwitch(lcpfw::kSwitchRelogin);

    base::LaunchProcess(command_line, launch_options);
}

void LogApplicationStartup()
{
    const char kStartupTag[] = "--- Main Startup ---";
    LOG(INFO) << kStartupTag;
}

void LogApplicationExit(int result_code)
{
    const char kNormalExitTag[] = "--- Main Exit ---";
    LOG(INFO) << kNormalExitTag << "\nExit result code: " << result_code;
}

}   // namespace


APP_LIB_EXPORT int AppMainEntry()
{
    int argc = 0;
    char **argv = nullptr;
    base::CommandLine::Init(argc, argv);

    base::AtExitManager exit_manager;

//#ifdef OS_WIN
//    // Ole must be initialized before starting message pump, so that TSF
//    // (Text Services Framework) module can interact with the message pump
//    // on Windows 8 Metro mode.
//    ui::ScopedOleInitializer ole_initializer;
//#endif

    base::EnableTerminationOnHeapCorruption();
    base::EnableTerminationOnOutOfMemory();
#ifdef OS_WIN
    base::Time::EnableHighResolutionTimer(true);
    base::Time::ActivateHighResolutionTimer(true);
#endif

    base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

    //// Disabling Direct Composition works around the limitation that
    //// InProcessContextFactory doesn't work with Direct Composition, causing the
    //// window to not render. See http://crbug.com/936249.
    //command_line->AppendSwitch(switches::kDisableDirectComposition);

    //// Disable skia renderer to use GL instead.
    //std::string disabled =
    //    command_line->GetSwitchValueASCII(switches::kDisableFeatures);
    //if (!disabled.empty())
    //    disabled += ",";
    //disabled += features::kUseSkiaRenderer.name;
    //command_line->AppendSwitchASCII(switches::kDisableFeatures, disabled);

    base::FeatureList::InitializeInstance(
        command_line->GetSwitchValueASCII(switches::kEnableFeatures),
        command_line->GetSwitchValueASCII(switches::kDisableFeatures));

    //mojo::core::Init();

    //gl::init::InitializeGLOneOff();
    //gl::AppGLSurfaceSupport::InitializeOneOff();

    // Viz depends on the task environment to correctly tear down.
    /*base::AppTaskEnvironment task_environment(
        base::AppTaskEnvironment::MainThreadType::UI);*/

    //// The ContextFactory must exist before any Compositors are created.
    //auto context_factories =
    //    std::make_unique<ui::AppContextFactories>(false);
    //context_factories->SetUseTestSurface(false);
    //viz::AppGpuServiceHolder::DoNotResetOnTestExit();

    base::i18n::InitializeICU();

    lcpfw::RegisterPathProvider();
    //ui::RegisterPathProvider();
    
    /*base::FilePath ui_test_pak_path;
    CHECK(base::PathService::Get(ui::UI_TEST_PAK, &ui_test_pak_path));
    ui::ResourceBundle::InitSharedInstanceWithPakPath(ui_test_pak_path);

    base::FilePath views_examples_resources_pak_path;
    CHECK(base::PathService::Get(base::DIR_MODULE,
                                &views_examples_resources_pak_path));
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        views_examples_resources_pak_path.AppendASCII(
            "views_examples_resources.pak"),
        ui::SCALE_FACTOR_100P);*/

//    gfx::InitializeFonts();
//
//#if defined(USE_AURA)
//    std::unique_ptr<aura::Env> env = aura::Env::CreateInstance();
//    aura::Env::GetInstance()->set_context_factory(
//        context_factories->GetContextFactory());
//#endif
//
//    ui::InitializeInputMethod();
//
//    //views::DesktopTestViewsDelegate views_delegate;
//
//#if defined(USE_AURA)
//    wm::WMState wm_state;
//#endif
////#if BUILDFLAG(ENABLE_DESKTOP_AURA)
////    std::unique_ptr<display::Screen> desktop_screen =
////        base::WrapUnique(views::CreateDesktopScreen());
////#endif

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

    LogApplicationStartup();

    std::unique_ptr<AppMainRunner> main_runner = AppMainRunner::Create();

    // If critical failures happened, like we couldn't even create worker threads, exit before
    // running into message loop.
    MainFunctionParams params(*command_line, base::Bind(CreateAppMainParts));
    int result_code = main_runner->Initialize(params);
    if (result_code >= lcpfw::ResultCodeErrorOccurred) {
        return result_code;
    }

    result_code = main_runner->Run();

    main_runner->Shutdown();

    if (result_code == lcpfw::ResultCodeRestartApp) {
        RestartApp();
    }

//    ui::ResourceBundle::CleanupSharedInstance();
//
//    ui::ShutdownInputMethod();
//
//#if defined(USE_AURA)
//    env.reset();
//#endif

    base::CommandLine::Reset();

    LogApplicationExit(result_code);

    return result_code;
}
