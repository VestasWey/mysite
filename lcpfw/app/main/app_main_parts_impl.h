#pragma once

#include <memory>

#include "base/command_line.h"
#include "base/files/file_path.h"

#include "content/app_main_extra_parts.h"
#include "content/app_main_parts.h"
#include "content/main_function_params.h"
#include "main/app_main_process_impl.h"
#include "main/app_single_instance_guarantor.h"
#include "common/profiles/profile.h"
//#include "main/ui/startup/startup_app_creator.h"


class AppMainPartsImpl final
    : public AppMainParts
{
public:
    AppMainPartsImpl(const MainFunctionParams& main_function_params);
    virtual ~AppMainPartsImpl();

    // Add additional AppMainExtraParts.
    void AddParts(std::unique_ptr<AppMainExtraParts> parts);

private:
    // content::BrowserMainParts overrides.
    // These are called in-order by content::BrowserMainLoop.
    // Each stage calls the same stages in any ChromeBrowserMainExtraParts added
    // with AddParts() from ChromeContentBrowserClient::CreateBrowserMainParts.
    int PreEarlyInitialization() override;
    void PostEarlyInitialization() override;
    void ToolkitInitialized() override;
    void PreMainMessageLoopStart() override;
    void PostMainMessageLoopStart() override;
    int PreCreateThreads() override;
    void PostCreateThreads() override;
    void PreMainMessageLoopRun() override;
    bool MainMessageLoopRun(int* result_code) override;
    void PostMainMessageLoopRun() override;
    void PostDestroyThreads() override;

    // Additional stages for ChromeBrowserMainExtraParts. These stages are called
    // in order from PreMainMessageLoopRun(). See implementation for details.
    virtual void PreProfileInit();
    virtual void PostProfileInit();
    virtual void PreAppStart();
    virtual void AppStart();
    virtual void PostAppStart();

private:
    // Methods for Main Message Loop -------------------------------------------
    int PreCreateThreadsImpl();
    int PreMainMessageLoopRunImpl();

    void OnLocalStateLoaded();

    const MainFunctionParams& parameters() const {
        return parameters_;
    }
    const base::CommandLine& parsed_command_line() const
    {
        return parsed_command_line_;
    }
    const base::FilePath& user_data_dir() const {
        return user_data_dir_;
    }

private:
    const MainFunctionParams parameters_;
    const base::CommandLine& parsed_command_line_;
    int result_code_;

    // Create ShutdownWatcherHelper object for watching jank during shutdown.
    // Please keep |shutdown_watcher| as the first object constructed, and hence
    // it is destroyed last.
    //std::unique_ptr<ShutdownWatcherHelper> shutdown_watcher_;

    std::vector<std::unique_ptr<AppMainExtraParts>> app_extra_parts_;

    std::unique_ptr<AppMainProcessImpl> app_process_;

    // Browser creation happens on the Java side in Android.
    //std::unique_ptr<StartupAppCreator> app_creator_;

    // Android doesn't support multiple browser processes, so it doesn't implement
    // ProcessSingleton.
    //std::unique_ptr<AppProcessSingleton> process_singleton_;

    // ProcessSingleton::NotifyResult notify_result_ =
    //     ProcessSingleton::PROCESS_NONE;

    // Members needed across shutdown methods.
    bool restart_last_session_ = false;

    //Profile* profile_;
    bool run_message_loop_ = true;

    base::FilePath user_data_dir_;

    bool app_started_ = false;

    DISALLOW_COPY_AND_ASSIGN(AppMainPartsImpl);
};

// for bind to MainFunctionParams.created_main_parts_closure
std::unique_ptr<AppMainParts> CreateAppMainParts(const MainFunctionParams& main_function_params);