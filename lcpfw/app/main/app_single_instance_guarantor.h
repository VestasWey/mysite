#pragma once

#include "base/callback.h"

class AppProcessSingleton
{
public:
    // Logged as histograms, do not modify these values.
    enum NotifyResult {
        PROCESS_NONE = 0,
        PROCESS_NOTIFIED = 1,
        PROFILE_IN_USE = 2,
        LOCK_ERROR = 3,
        LAST_VALUE = LOCK_ERROR
    };

public:
    // Implement this callback to handle notifications from other processes. The
    // callback will receive the command line and directory with which the other
    // Chrome process was launched. Return true if the command line will be
    // handled within the current browser instance or false if the remote process
    // should handle it (i.e., because the current process is shutting down).
    using NotificationCallback =
        base::RepeatingCallback<bool(const base::CommandLine& command_line,
                                    const base::FilePath& current_directory)>;

    AppProcessSingleton(const base::FilePath& user_data_dir,
                   const NotificationCallback& notification_callback);

    virtual ~AppProcessSingleton();

    // Notify another process, if available. Otherwise sets ourselves as the
    // singleton instance. Returns PROCESS_NONE if we became the singleton
    // instance. Callers are guaranteed to either have notified an existing
    // process or have grabbed the singleton (unless the profile is locked by an
    // unreachable process).
    // TODO(brettw): Make the implementation of this method non-platform-specific
    // by making Linux re-use the Windows implementation.
    NotifyResult NotifyOtherProcessOrCreate();

    // Sets ourself up as the singleton instance.  Returns true on success.  If
    // false is returned, we are not the singleton instance and the caller must
    // exit.
    // NOTE: Most callers should generally prefer NotifyOtherProcessOrCreate() to
    // this method, only callers for whom failure is preferred to notifying
    // another process should call this directly.
    bool Create();

    // Clear any lock state during shutdown.
    void Cleanup();

private:
    base::FilePath user_data_dir_;
    NotificationCallback notification_callback_;  // Handler for notifications.

    SEQUENCE_CHECKER(sequence_checker_);

    DISALLOW_COPY_AND_ASSIGN(AppProcessSingleton);
};
