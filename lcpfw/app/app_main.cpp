
//#include <windows.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <random>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/process/launch.h"
#include "base/process/process.h"

#include "app_installation_rejecter.h"
#include "common/app_result_codes.h"
#include "common/app_paths.h"
#include "common/app_logging.h"
#include "common/app_context.h"
#include "common/app_constants.h"
#include "main_dll_loader.h"


#if OS_WIN
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE hPrevInstance, wchar_t*, int)
#else
int main(int argc, char** argv)
#endif
{
#if OS_WIN
    base::CommandLine::Init(0, nullptr);
#else
    base::CommandLine::Init(argc, argv);
#endif
    base::AtExitManager exit_manager;

    lcpfw::RegisterPathProvider();
    lcpfw::InitAppLogging();
    AppContext::Current()->Init();

    auto cmdline = base::CommandLine::ForCurrentProcess();

    LOG(INFO) << "------------- App Startup ---------------";
    LOG(INFO) << cmdline->GetCommandLineString();

    // Reject installer cover setup.
    if (AppInstallationRejecter::Reject())
    {
        LOG(INFO) << "Installer is running now.";
        LOG(INFO) << "------------- App Exit ---------------";
        return lcpfw::ResultCodeErrorOccurred;
    }

    int result = 0;
    std::unique_ptr<AppDllLoader> loader;
    auto process_type = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(lcpfw::kSwitchProcessType);
    if (process_type.compare(lcpfw::kAppWatcher) == 0)
    {
        // Load CrashHandlerServer moudule
        loader = std::make_unique<AppDllLoader>(base::FilePath(lcpfw::kAppWatcherDll));
        //::MessageBox(nullptr, cmdline->GetCommandLineString().c_str(), (L"App Watcher " + std::to_wstring(base::Process::Current().Pid())).c_str(), MB_OK);
    }
    else
    {
#if NDEBUG
        // First, launch the crash-handler program, as an crash watcher, 
        // when app crashed it will show a message-box and allow user restart app, feedback how crash happened.
        base::CommandLine crashpad(cmdline->GetProgram());
        crashpad.AppendSwitchASCII(lcpfw::kSwitchProcessType, lcpfw::kAppWatcher);
        crashpad.AppendSwitchASCII(lcpfw::kSwitchParentPID, std::to_string(base::Process::Current().Pid()));
        base::LaunchProcess(crashpad, base::LaunchOptions());
#endif

        // Second, load the main dll
        loader = std::make_unique<MainDllLoader>(base::FilePath(lcpfw::kAppMainDll));
    }

    DCHECK(loader);
    if (loader)
    {
        result = loader->Launch();
        loader = nullptr;
    }

    base::CommandLine::Reset();

    LOG(INFO) << "------------- App Exit(" << result << ") ---------------";

    return result;
}

