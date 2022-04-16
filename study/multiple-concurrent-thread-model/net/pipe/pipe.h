#pragma once
#include <atomic>
#include <list>
#include <string>

#include "message_loop/message_pump.h"
#include "net/io_buffer_pool.h"

namespace mctm
{
    class ThreadChecker;

    struct AsyncContext
    {
        AsyncContext()
        {
            ResetIOContext();
        }

        void ResetIOContext()
        {
            memset(&overlapped, 0, sizeof(overlapped));
        }

        std::atomic_bool is_pending_ = false;
        MessagePumpForIO::IOContext overlapped;
        IOBuffer io_buffer;
    };

    class PipeDataTransfer
    {
    public:
        PipeDataTransfer();
        virtual ~PipeDataTransfer();

        virtual bool Read();
        virtual bool Write(const char* data, unsigned int len);
        virtual void Close();

    protected:
        HANDLE pipe_handle_ = INVALID_HANDLE_VALUE;
        AsyncContext read_io_context_;
        AsyncContext write_io_context_;
    };

    class PipeServer
    {
        class ClientInfo 
            : public PipeDataTransfer
            , public MessagePumpForIO::IOHandler
        {
        public:
            ClientInfo(HANDLE pipe_handle, PipeServer* pipe_server);
            virtual ~ClientInfo();

            bool Accept();
            AsyncType GetAsyncType(MessagePumpForIO::IOContext* context);
            HANDLE pipe_handle() const { return pipe_handle_; }

            // PipeDataTransfer
            void Close() override;

        protected:
            // IOHandler
            void OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered,
                DWORD error) override;

        private:
            PipeServer* pipe_server_ = nullptr;
            AsyncContext accept_io_context_;
        };
        using ScopedClient = std::unique_ptr<ClientInfo>;

    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            virtual void OnPipeServerAccept(ULONG_PTR client_key, DWORD error) = 0;
            virtual void OnPipeServerReadData(ULONG_PTR client_key,
                DWORD error, const char* data, unsigned int len) = 0;
            virtual void OnPipeServerWriteData(ULONG_PTR client_key,
                DWORD error, const char* data, unsigned int len) = 0;
            virtual void OnPipeServerError(ULONG_PTR client_key, DWORD error) = 0;
        };

        PipeServer(const std::wstring& pipe_name, Delegate* delegate, 
            unsigned int max_pipe_instances_count = 1, bool auto_supplement = true);
        virtual ~PipeServer();

        // must be called on io thread
        bool Start();
        bool Send(ULONG_PTR client_key, const char* data, unsigned int len);
        void Stop();

    private:
        void SupplementPipeInstance();
        ScopedClient Create();
        bool Listen(ClientInfo* client);
        bool Read(ClientInfo* client);
        bool Write(ClientInfo* client, const char* data, unsigned int len);

        void OnClientConnect(ClientInfo* client, DWORD error);
        void OnClientReadData(ClientInfo* client, DWORD error, const char* data, unsigned int len);
        void OnClientWriteData(ClientInfo* client, DWORD error, const char* data, unsigned int len);
        void OnClientError(ClientInfo* client, DWORD error);

    private:
        friend class ClientInfo;

        std::unique_ptr<ThreadChecker> thread_check_;
        Delegate* delegate_ = nullptr;
        std::wstring pipe_name_;
        unsigned int max_pipe_instances_count_ = 1;
        bool auto_supplement_ = true;
        std::list<ScopedClient> clients_;
        bool stop_ = true;
    };
    
    class PipeClient
        : public PipeDataTransfer
        , MessagePumpForIO::IOHandler
    {
    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            virtual void OnPipeClientConnect(PipeClient* client_key, DWORD error) = 0;
            virtual void OnPipeClientReadData(PipeClient* client_key,
                DWORD error, const char* data, unsigned int len) = 0;
            virtual void OnPipeClientWriteData(PipeClient* client_key,
                DWORD error, const char* data, unsigned int len) = 0;
            virtual void OnPipeClientError(PipeClient* client_key, DWORD error) = 0;
        };

        PipeClient(const std::wstring& pipe_name, Delegate* delegate);
        virtual ~PipeClient();

        // must be called on io thread
        bool Connect();
        bool Send(const char* data, unsigned int len);
        void Close() override; // PipeDataTransfer

    protected:
        AsyncType GetAsyncType(MessagePumpForIO::IOContext* context);

        // IOHandler
        void OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered,
            DWORD error) override;

    private:
        std::unique_ptr<ThreadChecker> thread_check_;
        Delegate* delegate_ = nullptr;
        std::wstring pipe_name_;
    };
}