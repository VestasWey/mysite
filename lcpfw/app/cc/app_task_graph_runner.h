// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/threading/simple_thread.h"
#include "cc/raster/single_thread_task_graph_runner.h"

namespace cc {

class AppTaskGraphRunner : public SingleThreadTaskGraphRunner {
 public:
  AppTaskGraphRunner();
  AppTaskGraphRunner(const AppTaskGraphRunner&) = delete;
  ~AppTaskGraphRunner() override;

  AppTaskGraphRunner& operator=(const AppTaskGraphRunner&) = delete;
};

}  // namespace cc
