#pragma once

#include "base/process/process_handle.h"

std::string GetCrashInfoFileName(base::ProcessId client_pid);
std::string GetCrashLogFileName(base::ProcessId client_pid);
bool WaitForCrashServerReady();