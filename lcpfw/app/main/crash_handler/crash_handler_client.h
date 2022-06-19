#pragma once

#include <memory>

#include "base/files/file_path.h"

namespace google_breakpad {
    class ExceptionHandler;
}


class CrashHandlerClient {
public:
    CrashHandlerClient();

    ~CrashHandlerClient();

    void Install();

    // Normally you shouldn't call this function on your own.
    // Just leave it to decree of the class itself.
    void UnInstall();

private:
    DISALLOW_COPY_AND_ASSIGN(CrashHandlerClient);

private:
    std::unique_ptr<google_breakpad::ExceptionHandler> exception_handler_;
};
