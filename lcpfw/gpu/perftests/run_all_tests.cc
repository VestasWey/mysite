// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/message_loop/message_pump_type.h"
#include "base/task/single_thread_task_executor.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "base/test/test_suite.h"
#include "build/build_config.h"
#include "ui/gl/init/gl_factory.h"

#if defined(USE_OZONE)
#include "ui/base/ui_base_features.h"  // nogncheck
#include "ui/ozone/public/ozone_platform.h"
#endif

static int RunHelper(base::TestSuite* test_suite) {
  base::FeatureList::InitializeInstance(std::string(), std::string());
  std::unique_ptr<base::SingleThreadTaskExecutor> executor;
#if defined(USE_OZONE)
  if (features::IsUsingOzonePlatform()) {
    executor = std::make_unique<base::SingleThreadTaskExecutor>(
        base::MessagePumpType::UI);
    ui::OzonePlatform::InitParams params;
    params.single_process = true;
    ui::OzonePlatform::InitializeForGPU(params);
  } else
#endif
  {
    executor = std::make_unique<base::SingleThreadTaskExecutor>(
        base::MessagePumpType::IO);
  }

  CHECK(gl::init::InitializeGLOneOff());
  return test_suite->Run();
}

int main(int argc, char** argv) {
  base::TestSuite test_suite(argc, argv);
  base::CommandLine::Init(argc, argv);

  // Always run the perf tests serially, to avoid distorting
  // perf measurements with randomness resulting from running
  // in parallel.
  return base::LaunchUnitTestsSerially(
      argc, argv, base::BindOnce(&RunHelper, base::Unretained(&test_suite)));
}
