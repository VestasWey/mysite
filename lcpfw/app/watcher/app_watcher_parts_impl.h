#pragma once

#include <memory>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/synchronization/waitable_event.h"

#include "content/app_main_extra_parts.h"
#include "content/app_main_parts.h"
#include "content/main_function_params.h"
#include "common/profiles/profile.h"

class CrashHandlerServer;

namespace content {
    class NotificationService;
}

class WatcherMainPartsImpl final
    : public AppMainParts
{
public:
    WatcherMainPartsImpl(const MainFunctionParams& main_function_params);
    virtual ~WatcherMainPartsImpl();

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

    void OnParentProcessExit();

private:
    const MainFunctionParams parameters_;
    const base::CommandLine& parsed_command_line_;
    int result_code_;

    std::unique_ptr<CrashHandlerServer> crash_handler_server_;

    //std::unique_ptr<content::NotificationService> ntf_service_;

    std::vector<std::unique_ptr<AppMainExtraParts>> app_extra_parts_;

    // Members needed across shutdown methods.
    bool restart_last_session_ = false;

    bool run_message_loop_ = true;

    base::FilePath user_data_dir_;

    DISALLOW_COPY_AND_ASSIGN(WatcherMainPartsImpl);
};

// for bind to MainFunctionParams.created_main_parts_closure
std::unique_ptr<AppMainParts> CreateAppMainParts(const MainFunctionParams& main_function_params);