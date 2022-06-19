// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BROWSER_COMMAND_CONTROLLER_H_
#define CHROME_BROWSER_UI_BROWSER_COMMAND_CONTROLLER_H_

#include <vector>

#include "base/macros.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_member.h"

#include "main/ui/command_updater.h"
#include "main/ui/command_updater_delegate.h"
#include "main/ui/command_updater_impl.h"

class MainModule;
class Profile;

// This class needs to expose the internal command_updater_ in some way, hence
// it implements CommandUpdater as the public API for it (so it's not directly
// exposed).
class CommandController : public CommandUpdater
{
 public:
  CommandController();
  ~CommandController() override;

  void SetMainModule(MainModule* main_module);

  // Override from CommandUpdater:
  bool SupportsCommand(int id) const override;
  bool IsCommandEnabled(int id) const override;
  bool ExecuteCommand(
      int id,
      base::TimeTicks time_stamp = base::TimeTicks::Now()) override;
  bool ExecuteCommandWithParams(
      int id,
      const CommandParamsDetails& params,
      base::TimeTicks time_stamp = base::TimeTicks::Now()) override;
  bool UpdateCommandEnabled(int id, bool state) override;

 private:
  // Initialize state for all browser commands.
  void InitCommandState();

  MainModule* main_module_ = nullptr;

  // The CommandUpdaterImpl that manages the browser window commands.
  CommandUpdaterImpl command_updater_;

  PrefChangeRegistrar profile_pref_registrar_;
  PrefChangeRegistrar local_pref_registrar_;
  BooleanPrefMember pref_signin_allowed_;

  DISALLOW_COPY_AND_ASSIGN(CommandController);
};

#endif  // CHROME_BROWSER_UI_BROWSER_COMMAND_CONTROLLER_H_
