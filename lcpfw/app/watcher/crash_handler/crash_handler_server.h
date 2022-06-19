#pragma once

#include <memory>
#include "client/windows/crash_generation/crash_generation_server.h"


//namespace google_breakpad {
//    class ClientInfo;
//    class CrashGenerationServer;
//}

class CrashHandlerServer {
public:
    CrashHandlerServer(base::RepeatingClosure quit_closure);

    ~CrashHandlerServer() = default;

    bool Start();

    void Stop();

private:
    static void OnClientConnected(void* context, const google_breakpad::ClientInfo* client_info);

    static void OnClientCrashed(void* context, const google_breakpad::ClientInfo* client_info,
        const std::wstring* dump_path);

    static void OnClientExited(void* context, const google_breakpad::ClientInfo* client_info);

private:
    std::unique_ptr<google_breakpad::CrashGenerationServer> server_;

    DISALLOW_COPY_AND_ASSIGN(CrashHandlerServer);
};

