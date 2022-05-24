#pragma once

#include <memory>

#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/sequence_checker.h"

#include "common/profiles/profile.h"


class AppMainProcess
{
public:
    // Invoked when the user is logging out/shutting down. When logging off we may
    // not have enough time to do a normal shutdown. This method is invoked prior
    // to normal shutdown and saves any state that must be saved before system
    // shutdown.
    virtual void EndSession() = 0;

    // Ensures |local_state()| was flushed to disk and then posts |reply| back on
    // the current sequence.
    virtual void FlushLocalStateAndReply(base::OnceClosure reply) = 0;

    virtual bool IsShuttingDown() = 0;

    // Returns the locale used by the application. It is the IETF language tag,
    // defined in BCP 47. The region subtag is not included when it adds no
    // distinguishing information to the language tag (e.g. both "en-US" and "fr"
    // are correct here).
    virtual const std::string& GetApplicationLocale() = 0;
    virtual void SetApplicationLocale(const std::string& actual_locale) = 0;

    virtual Profile* global_profile() = 0;
    virtual Profile* profile() = 0;
    virtual PrefService* global_state() = 0;
    virtual PrefService* local_state() = 0;

    // Returns the StatusTray, which provides an API for displaying status icons
    // in the system status tray. Returns NULL if status icons are not supported
    // on this platform (or this is a unit test).
    //virtual StatusTray* status_tray() = 0;

protected:
    AppMainProcess() = default;
    virtual ~AppMainProcess() = default;
};

// Impl by AppMainProcessImpl or other derive class
AppMainProcess* GetAppMainProcess();