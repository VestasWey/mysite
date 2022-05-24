// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_util.h"

#include "main_dll_loader.h"
#include "common/app_logging.h"
#include "common/app_context.h"
#include "public/common/app_constants.h"
#include "public/common/app_paths.h"

namespace {

}  // namespace

namespace {

//int RunMain(int argc, char* argv[]) {
//
//  return 0;
//}

}  // namespace

// Entry point function for the browser process.
int main(int argc, char* argv[])
{
    int rc = 0;
    base::AtExitManager exit_manager;
    base::CommandLine::Init(argc, argv);
    
    lcpfw::RegisterPathProvider();
    lcpfw::InitAppLogging();
    AppContext::Current()->Init();

    LOG(INFO) << "------------- App Startup ---------------";
    LOG(INFO) << AppContext::Current()->GetExecutablePath().AsUTF8Unsafe();

    auto process_type = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(lcpfw::kSwitchProcessType);

    std::unique_ptr<MainDllLoader> loader(MakeMainDllLoader());
    rc = loader->Launch();

    base::CommandLine::Reset();
    return rc;
}
