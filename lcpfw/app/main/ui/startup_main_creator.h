#pragma once


#include "base/files/file_path.h"
#include "base/command_line.h"


class AppMainProcessImpl;

class StartupMainCreator
{
public:
    StartupMainCreator(AppMainProcessImpl* process_impl);
    ~StartupMainCreator();

    bool Start(const base::CommandLine& cmd_line);

private:
    bool ProcessCmdLineImpl(const base::CommandLine& command_line);

    bool LaunchAppMain(const base::CommandLine& command_line);
    bool LaunchExamples(const base::CommandLine& command_line);

private:
    AppMainProcessImpl* process_impl_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(StartupMainCreator);
};
