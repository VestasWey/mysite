// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BROWSER_COMMANDS_H_
#define CHROME_BROWSER_UI_BROWSER_COMMANDS_H_

#include <string>
#include <vector>

#include "base/optional.h"
#include "base/time/time.h"
#include "build/build_config.h"

#include "main/ui/command_updater_delegate.h"
#include "public/main/command_ids.h"

namespace lcpfw {

bool IsCommandEnabled(int command);
bool SupportsCommand(int command);
bool ExecuteCommand(int command, base::TimeTicks time_stamp = base::TimeTicks::Now());
bool ExecuteCommandWithParams(int command, const CommandParamsDetails& params);
bool UpdateCommandEnabled(int command, bool enabled);

}  // namespace lcpfw

#endif  // CHROME_BROWSER_UI_BROWSER_COMMANDS_H_
