#pragma once

#include <memory>

#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/sequence_checker.h"
//#include "components/keep_alive_registry/app_keep_alive_state_observer.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_registry_simple.h"

#include "common/profiles/profile.h"
#include "content/app_main_process.h"

namespace base {

class SequencedTaskRunner;
class ScopedNativeLibrary;
}

class AppMainPartsImpl;
class AppSecret;

class AppMainProcessImpl
    : public AppMainProcess/*,
    public KeepAliveStateObserver*/
{
public:
    static void RegisterGlobalPrefs(PrefRegistrySimple* registry);
    static void RegisterUserPrefs(PrefRegistrySimple* registry);

    // Called to complete initialization.
    void Init();

    // Sets a closure to be run to break out of a run loop on browser shutdown
    // (when the KeepAlive count reaches zero).
    // TODO(https://crbug.com/845966): This is also used on macOS for the Cocoa
    // first run dialog so that shutdown can be initiated via a signal while the
    // first run dialog is showing.
    void SetQuitClosure(base::OnceClosure quit_closure);

#if defined(OS_MAC)
    // Clears the quit closure. Shutdown will not be initiated should the
    // KeepAlive count reach zero. This function may be called more than once.
    // TODO(https://crbug.com/845966): Remove this once the Cocoa first run
    // dialog no longer needs it.
    void ClearQuitClosure();
#endif

    // AppProcess
    bool IsShuttingDown() override;
    void SetApplicationLocale(const std::string& locale) override;
    const std::string& GetApplicationLocale() override;
    Profile* global_profile() override;
    Profile* profile() override;
    PrefService* local_state() override;
    PrefService* global_state() override;
    //StatusTray* status_tray() override;

private:
    AppMainProcessImpl(const base::FilePath& user_data_dir);

    virtual ~AppMainProcessImpl();

    // Called before the browser threads are created.
    void PreCreateThreads();

    // Called after the threads have been created but before the message loops
    // starts running. Allows the browser process to do any initialization that
    // requires all threads running.
    bool PreMainMessageLoopRun();

    // Most cleanup is done by these functions, driven from
    // ChromeBrowserMain based on notifications from the content
    // framework, rather than in the destructor, so that we can
    // interleave cleanup with threads being stopped.
    void StartTearDown();
    void PostDestroyThreads();

    // BrowserProcess implementation.
    void EndSession() override;
    void FlushLocalStateAndReply(base::OnceClosure reply) override;

    // KeepAliveStateObserver implementation
    //void OnKeepAliveStateChanged(bool is_keeping_alive) override;
    //void OnKeepAliveRestartStateChanged(bool can_restart) override;

    // Methods called to control our lifetime. The browser process can be "pinned"
    // to make sure it keeps running.
    void Pin();
    void Unpin();

    void InitGlobalProfile();
    void InitLocalProfile();

    bool LoadSecretModule();

private:
    friend struct std::default_delete<AppMainProcessImpl>;
    friend class AppMainPartsImpl;

    //std::unique_ptr<StatusTray> status_tray_;

    bool shutting_down_ = false;
    bool tearing_down_ = false;

    std::string locale_;

    // Ensures that the observers of plugin/print disable/enable state
    // notifications are properly added and removed.
    PrefChangeRegistrar pref_change_registrar_;

    // Called to signal the process' main message loop to exit.
    base::OnceClosure quit_closure_;

    std::unique_ptr<Profile> global_profile_;
    // init until user login success
    std::unique_ptr<Profile> profile_;
    // associate with global thread-pool, use for prefs write
    scoped_refptr<base::SingleThreadTaskRunner> profile_task_runner_;

    base::debug::StackTrace release_last_reference_callstack_;
    base::FilePath user_data_dir_;

    std::unique_ptr<base::ScopedNativeLibrary> secret_dll_;
    scoped_refptr<AppSecret> secret_module_;

    SEQUENCE_CHECKER(sequence_checker_);

    DISALLOW_COPY_AND_ASSIGN(AppMainProcessImpl);
};
