// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "main/ui/command_handler.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/user_metrics.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"

#include "content/app_main_process.h"
#include "main/ui/command_controller.h"

namespace lcpfw {

    bool IsCommandEnabled(int command)
    {
        if (!GetAppMainProcess() || !GetAppMainProcess()->command_controller())
        {
            return false;
        }

        return GetAppMainProcess()->command_controller()->IsCommandEnabled(command);
    }

    bool SupportsCommand(int command)
    {
        if (!GetAppMainProcess() || !GetAppMainProcess()->command_controller())
        {
            return false;
        }

        return GetAppMainProcess()->command_controller()->SupportsCommand(command);
    }

    bool ExecuteCommand(int command, base::TimeTicks time_stamp)
    {
        if (!GetAppMainProcess() || !GetAppMainProcess()->command_controller())
        {
            return false;
        }

        return GetAppMainProcess()->command_controller()->ExecuteCommand(command, time_stamp);
    }

    bool ExecuteCommandWithParams(int command,
        const CommandParamsDetails& params)
    {
        if (!GetAppMainProcess() || !GetAppMainProcess()->command_controller())
        {
            return false;
        }

        return GetAppMainProcess()->command_controller()->ExecuteCommandWithParams(
            command, params);
    }

    bool UpdateCommandEnabled(int command, bool enabled)
    {
        if (!GetAppMainProcess() || !GetAppMainProcess()->command_controller())
        {
            return false;
        }

        return GetAppMainProcess()->command_controller()->UpdateCommandEnabled(command, enabled);
    }

}  // namespace lcpfw
