
//#include <windows.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <random>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "app_installation_rejecter.h"
#include "common/app_result_codes.h"
#include "common/app_paths.h"
#include "common/app_logging.h"
#include "common/app_context.h"
#include "common/app_constants.h"
#include "main_dll_loader.h"

int main(int argc, char* argv[])
{
    base::CommandLine::Init(0, nullptr);
    base::AtExitManager exit_manager;

    if (AppInstallationRejecter::Reject()) {
        return lcpfw::ResultCodeErrorOccurred;
    }

    lcpfw::RegisterPathProvider();
    lcpfw::InitAppLogging();
    AppContext::Current()->Init();

    LOG(INFO) << "------------- App Startup ---------------";
    LOG(INFO) << AppContext::Current()->GetExecutablePath().AsUTF8Unsafe();

    auto process_type = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(lcpfw::kSwitchProcessType);

    std::unique_ptr<MainDllLoader> loader(MakeMainDllLoader());
    int rc = loader->Launch();

    base::CommandLine::Reset();

    LOG(INFO) << "------------- App Exit ---------------";

    return rc;
}

