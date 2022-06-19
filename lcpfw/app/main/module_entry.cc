
#ifdef OS_WIN
#include <Windows.h>
#endif

#include "base/at_exit.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/process/memory.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "components/viz/common/features.h"
#include "ui/gl/gl_switches.h"

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


APP_LIB_EXPORT int AppModuleEntry()
{
    int argc = 0;
    char **argv = nullptr;
    base::CommandLine::Init(argc, argv);

    base::AtExitManager exit_manager;

    base::EnableTerminationOnHeapCorruption();
    base::EnableTerminationOnOutOfMemory();
#ifdef OS_WIN
    base::Time::EnableHighResolutionTimer(true);
    base::Time::ActivateHighResolutionTimer(true);
#endif

    base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

    // Disabling Direct Composition works around the limitation that
    // InProcessContextFactory doesn't work with Direct Composition, causing the
    // window to not render. See http://crbug.com/936249.
    command_line->AppendSwitch(switches::kDisableDirectComposition);

    // Disable skia renderer to use GL instead.
    std::string disabled =
        command_line->GetSwitchValueASCII(switches::kDisableFeatures);
    if (!disabled.empty())
        disabled += ",";
    disabled += features::kUseSkiaRenderer.name;
    command_line->AppendSwitchASCII(switches::kDisableFeatures, disabled);

    base::FeatureList::InitializeInstance(
        command_line->GetSwitchValueASCII(switches::kEnableFeatures),
        command_line->GetSwitchValueASCII(switches::kDisableFeatures));

    base::i18n::InitializeICU();

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

    base::CommandLine::Reset();

    LogApplicationExit(result_code);

    return result_code;
}
