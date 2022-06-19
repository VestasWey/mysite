// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "main/ui/command_updater_impl.h"

#include <algorithm>


class CommandUpdaterImpl::Command {
 public:
  bool enabled;

  Command() : enabled(true) {}
};

CommandUpdaterImpl::CommandUpdaterImpl(CommandUpdaterDelegate* delegate)
    : delegate_(delegate) {
}

CommandUpdaterImpl::~CommandUpdaterImpl() {
}

bool CommandUpdaterImpl::SupportsCommand(int id) const {
  return commands_.find(id) != commands_.end();
}

bool CommandUpdaterImpl::IsCommandEnabled(int id) const {
  auto command = commands_.find(id);
  if (command == commands_.end())
    return false;
  return command->second->enabled;
}

bool CommandUpdaterImpl::ExecuteCommand(int id, base::TimeTicks time_stamp) {
  return ExecuteCommandWithParams(id, EmptyCommandParams(),
                                       time_stamp);
}

bool CommandUpdaterImpl::ExecuteCommandWithParams(
    int id,
    const CommandParamsDetails& params,
    base::TimeTicks time_stamp) {
  if (SupportsCommand(id) && IsCommandEnabled(id)) {
    delegate_->ExecuteCommandWithParams(id, params);
    return true;
  }
  return false;
}

bool CommandUpdaterImpl::UpdateCommandEnabled(int id, bool enabled) {
  Command* command = GetCommand(id, true);
  if (command->enabled == enabled)
    return true;  // Nothing to do.
  command->enabled = enabled;
  return true;
}

void CommandUpdaterImpl::DisableAllCommands() {
  for (const auto& command_pair : commands_)
    UpdateCommandEnabled(command_pair.first, false);
}

std::vector<int> CommandUpdaterImpl::GetAllIds() {
  std::vector<int> result;
  for (const auto& command_pair : commands_)
    result.push_back(command_pair.first);
  return result;
}

CommandUpdaterImpl::Command*
CommandUpdaterImpl::GetCommand(int id, bool create) {
  bool supported = SupportsCommand(id);
  if (supported)
    return commands_[id].get();

  DCHECK(create);
  std::unique_ptr<Command>& entry = commands_[id];
  entry = std::make_unique<Command>();
  return entry.get();
}
