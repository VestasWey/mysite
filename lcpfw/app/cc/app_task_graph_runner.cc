// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/app_task_graph_runner.h"

namespace cc {

AppTaskGraphRunner::AppTaskGraphRunner() {
  Start("AppTaskGraphRunner", base::SimpleThread::Options());
}

AppTaskGraphRunner::~AppTaskGraphRunner() {
  Shutdown();
}

}  // namespace cc
