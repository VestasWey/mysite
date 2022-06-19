#include "startup_main_creator.h"

#include "base/command_line.h"

#include "common/app_constants.h"
#include "main/app_main_process_impl.h"
#include "main/ui/main_module.h"

StartupMainCreator::StartupMainCreator(AppMainProcessImpl* process_impl)
    : process_impl_(process_impl)
{
}

StartupMainCreator::~StartupMainCreator()
{
}

bool StartupMainCreator::Start(const base::CommandLine& cmd_line)
{
    return ProcessCmdLineImpl(cmd_line);
}

bool StartupMainCreator::ProcessCmdLineImpl(
    const base::CommandLine &command_line)
{
    // performs command-line handling then determine witch module should be actual startup
    //////////////////////////////////////////////////////////////////////////

    if (command_line.HasSwitch(lcpfw::kSwitchLaunchUIExamples))
    {
        return LaunchExamples(command_line);
    }

    return LaunchAppMain(command_line);
}

bool StartupMainCreator::LaunchAppMain(const base::CommandLine& command_line)
{
    scoped_refptr<MainModule> main_module(new MainModule());
    process_impl_->SetMainModule(main_module);

    main_module->Init();

    return true;
}

bool StartupMainCreator::LaunchExamples(const base::CommandLine& command_line)
{
    return true;
}
