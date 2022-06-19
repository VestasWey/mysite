// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "main/ui/command_controller.h"

#include <stddef.h>

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/containers/contains.h"
#include "base/debug/debugging_buildflags.h"
#include "base/debug/profiler.h"
#include "base/macros.h"
#include "build/branding_buildflags.h"
#include "build/build_config.h"

#include "main/ui/main_module.h"
#include "public/main/command_ids.h"

///////////////////////////////////////////////////////////////////////////////
// CommandController, public:

CommandController::CommandController()
    : command_updater_(nullptr)
{
    InitCommandState();
}

CommandController::~CommandController()
{
}


void CommandController::SetMainModule(MainModule* main_module)
{
    main_module_ = main_module;
}

////////////////////////////////////////////////////////////////////////////////
// CommandController, CommandUpdater implementation:

bool CommandController::SupportsCommand(int id) const
{
    return command_updater_.SupportsCommand(id);
}

bool CommandController::IsCommandEnabled(int id) const
{
    return command_updater_.IsCommandEnabled(id);
}

bool CommandController::ExecuteCommand(int id,
    base::TimeTicks time_stamp)
{
    return ExecuteCommandWithParams(id, EmptyCommandParams(), time_stamp);
}

bool CommandController::ExecuteCommandWithParams(
    int id,
    const CommandParamsDetails& params,
    base::TimeTicks time_stamp)
{
    DCHECK(command_updater_.IsCommandEnabled(id))
        << "Invalid/disabled command " << id;

    // Doesn't go through the command_updater_ to avoid dealing with having a
    // naming collision for ExecuteCommandWithDisposition (both
    // CommandUpdaterDelegate and CommandUpdater declare this function so we
    // choose to not implement CommandUpdaterDelegate inside this class and
    // therefore command_updater_ doesn't have the delegate set).
    if (!SupportsCommand(id) || !IsCommandEnabled(id))
        return false;

    switch (id)
    {
    case CMD_ACTIVE:
        break;
    case CMD_EXIT:
        break;
    default:
        LOG(WARNING) << "Received Unimplemented Command: " << id;
        break;
    }

    return true;
}

bool CommandController::UpdateCommandEnabled(int id, bool state)
{
    return command_updater_.UpdateCommandEnabled(id, state);
}

void CommandController::InitCommandState()
{
    // All browser commands whose state isn't set automagically some other way
    // (like Back & Forward with initial page load) must have their state
    // initialized here, otherwise they will be forever disabled.

    // Navigation commands
    command_updater_.UpdateCommandEnabled(CMD_ACTIVE, true);
    command_updater_.UpdateCommandEnabled(CMD_EXIT, true);
}

//void CommandController::UpdateCommandsForIncognitoAvailability() {
//  if (is_locked_fullscreen_)
//    return;
//
//  UpdateSharedCommandsForIncognitoAvailability(&command_updater_, profile());
//
//  if (!IsShowingMainUI()) {
//    command_updater_.UpdateCommandEnabled(IDC_IMPORT_SETTINGS, false);
//    command_updater_.UpdateCommandEnabled(IDC_OPTIONS, false);
//  }
//}
