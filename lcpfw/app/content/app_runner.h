#pragma once

#include "base/command_line.h"

#include "content/main_function_params.h"

class AppMainRunner
{
public:
    // Create a new AppMainRunner object.
    static std::unique_ptr<AppMainRunner> Create();

    // Returns true if the AppMainRunner has exited the main loop.
    static bool ExitedMainMessageLoop();


    virtual ~AppMainRunner() {}

    virtual int Initialize(const MainFunctionParams& params) = 0;

    virtual int Run() = 0;

    virtual void Shutdown() = 0;
};
