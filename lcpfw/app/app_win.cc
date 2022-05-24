// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>

#include "base/at_exit.h"
#include "base/command_line.h"

#include "common/app_logging.h"
#include "common/app_context.h"
#include "common/app_result_codes.h"
#include "exec/app_installation_rejecter.h"
#include "exec/main_dll_loader.h"
#include "public/common/app_constants.h"
#include "public/common/app_paths.h"


// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine,
    int nCmdShow)
{
    /*UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(nCmdShow);*/

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
    return rc;
}
