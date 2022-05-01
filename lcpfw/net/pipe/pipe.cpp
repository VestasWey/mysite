#include "pipe.h"

#include "logging/logging.h"
#include "message_loop/message_loop.h"
#include "threading/thread_checker.h"

namespace
{
    static const size_t kMaximumMessageSize = 128 * 1024 * 1024;
}

namespace mctm
{
    // PipeDataTransfer
    PipeDataTransfer::PipeDataTransfer()
    {
    }

    PipeDataTransfer::~PipeDataTransfer()
    {
        Close();
    }

    bool PipeDataTransfer::Read()
    {
        if ((INVALID_HANDLE_VALUE != pipe_handle_) &&
            (!read_io_context_.is_pending_))
        {
            read_io_context_.ResetIOContext();
            read_io_context_.is_pending_ = true;

            BOOL ok = ::ReadFile(pipe_handle_,
                read_io_context_.io_buffer.buffer,
                kIOBufferSize,
                nullptr,
                &read_io_context_.overlapped);
            if (!ok)
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING)
                {
                    return true;
                }
                DLOG(ERROR) << "pipe read error: " << err;
                return false;
            }
            return true;
        }
        return false;
    }

    bool PipeDataTransfer::Write(const char* data, unsigned long len)
    {
        if (len > kIOBufferSize)
        {
            DLOG(ERROR) << "write data out of memory";
            return false;
        }

        if ((INVALID_HANDLE_VALUE != pipe_handle_) &&
            (!write_io_context_.is_pending_))
        {
            write_io_context_.ResetIOContext();
            memcpy(write_io_context_.io_buffer.buffer, data, len);
            write_io_context_.is_pending_ = true;

            BOOL ok = ::WriteFile(pipe_handle_,
                write_io_context_.io_buffer.buffer,
                len,
                nullptr,
                &write_io_context_.overlapped);
            if (!ok)
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING)
                {
                    return true;
                }
                DLOG(ERROR) << "pipe write error: " << err;
                return false;
            }
            return true;
        }
        return false;
    }

    void PipeDataTransfer::Close()
    {
        if (pipe_handle_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(pipe_handle_);
            pipe_handle_ = INVALID_HANDLE_VALUE;
        }
    }


    // PipeServer
    PipeServer::PipeServer(const std::wstring& pipe_name, Delegate* delegate,
        unsigned int max_pipe_instances_count/* = 1*/, bool auto_supplement/* = true*/)
        : pipe_name_(pipe_name)
        , delegate_(delegate)
        , max_pipe_instances_count_(max_pipe_instances_count)
        , auto_supplement_(auto_supplement)
    {
    }

    PipeServer::~PipeServer()
    {
        Stop();
    }

    bool PipeServer::Start()
    {
        MessageLoopForIO* io_message_loop = MessageLoopForIO::current();
        if (!io_message_loop)
        {
            NOTREACHED() << "must be called on exists io thread";
            return false;
        }

        DCHECK(!thread_check_) << "should not called more than once";
        if (!thread_check_)
        {
            thread_check_ = std::make_unique<ThreadChecker>();
        }

        stop_ = false;
        for (unsigned int i = 0; i < max_pipe_instances_count_; ++i)
        {
            SupplementPipeInstance();
        }

        DCHECK(!clients_.empty());
        return !clients_.empty();
    }

    void PipeServer::Stop()
    {
        if (thread_check_)
        {
            DCHECK(thread_check_->CalledOnValidThread());
        }

        stop_ = true;
        for (auto& iter : clients_)
        {
            iter->Close();
        }

        clients_.clear();
        thread_check_.reset();
    }

    bool PipeServer::Send(ULONG_PTR client_key, const char* data, unsigned long len)
    {
        if (thread_check_)
        {
            DCHECK(thread_check_->CalledOnValidThread());
        }

        auto iter = std::find_if(clients_.begin(), clients_.end(), [&](ScopedClient& client)->bool
        {
            return (reinterpret_cast<ULONG_PTR>(client.get()) == client_key);
        });
        if (iter != clients_.end())
        {
            return (*iter)->Write(data, len);
        }
        return false;
    }

    void PipeServer::SupplementPipeInstance()
    {
        if (clients_.size() < max_pipe_instances_count_)
        {
            PipeServer::ScopedClient client = Create();
            if (client)
            {
                if (MessageLoopForIO::current()->RegisterIOHandler(client->pipe_handle(), client.get()))
                {
                    if (Listen(client.get()))
                    {
                        clients_.push_back(std::move(client));
                    }
                }
            }
        }
    }

    PipeServer::ScopedClient PipeServer::Create()
    {
        DWORD open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
        if (clients_.empty())
        {
            open_mode |= FILE_FLAG_FIRST_PIPE_INSTANCE;
        }

        HANDLE pipe_handle = ::CreateNamedPipeW(pipe_name_.c_str(),
            open_mode,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
            max_pipe_instances_count_,
            kIOBufferSize,
            kIOBufferSize,
            5000,
            nullptr);
        if (pipe_handle == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            DLOG(ERROR) << "PipeServer Create failed, code = " << err;
            return nullptr;
        }
        ScopedClient client = std::make_unique<ClientInfo>(pipe_handle, this);
        return client;
    }

    bool PipeServer::Listen(ClientInfo* client)
    {
        return client->Accept();
    }

    bool PipeServer::Read(ClientInfo* client)
    {
        return client->Read();
    }

    bool PipeServer::Write(ClientInfo* client, const char* data, unsigned int len)
    {
        return client->Write(data, len);
    }

    void PipeServer::OnClientConnect(ClientInfo* client, DWORD error)
    {
        if (delegate_)
        {
            delegate_->OnPipeServerAccept(reinterpret_cast<ULONG_PTR>(client), error);
        }
    }

    void PipeServer::OnClientReadData(ClientInfo* client, DWORD error, const char* data, unsigned int len)
    {
        if (delegate_)
        {
            delegate_->OnPipeServerReadData(reinterpret_cast<ULONG_PTR>(client), error, data, len);
        }
    }

    void PipeServer::OnClientWriteData(ClientInfo* client, DWORD error, const char* data, unsigned int len)
    {
        if (delegate_)
        {
            delegate_->OnPipeServerWriteData(reinterpret_cast<ULONG_PTR>(client), error, data, len);
        }
    }

    void PipeServer::OnClientError(ClientInfo* client, DWORD error)
    {
        clients_.remove_if([&](const ScopedClient& iter_client)->bool
        {
            return (iter_client.get() == client);
        });

        // 断开了一个实例，就再补充一个继续监听
        if (!stop_ && auto_supplement_)
        {
            SupplementPipeInstance();
        }

        if (delegate_)
        {
            delegate_->OnPipeServerError(reinterpret_cast<ULONG_PTR>(client), error);
        }
    }

    // PipeServer::ClientInfo
    PipeServer::ClientInfo::ClientInfo(HANDLE pipe_handle, PipeServer* pipe_server)
        : pipe_server_(pipe_server)
    {
        pipe_handle_ = pipe_handle;
    }

    PipeServer::ClientInfo::~ClientInfo()
    {
        Close();
    }

    void PipeServer::ClientInfo::Close()
    {
        if (pipe_handle_ != INVALID_HANDLE_VALUE)
        {
            if (accept_io_context_.is_pending_ ||
                read_io_context_.is_pending_ ||
                write_io_context_.is_pending_)
            {
                ::CancelIo(pipe_handle_);
            }

            ::DisconnectNamedPipe(pipe_handle_);

            ::CloseHandle(pipe_handle_);
            pipe_handle_ = INVALID_HANDLE_VALUE;

            // 主动循环IOCP，对所有在该pipe上投递的io操作进行处理，
            // 用以确保与pipe相关的资源都能得到处理，这样才能放心进行资源回收/释放管理
            DCHECK(MessageLoopForIO::current());
            while (accept_io_context_.is_pending_ ||
                read_io_context_.is_pending_ ||
                write_io_context_.is_pending_)
            {
                MessageLoopForIO::current()->WaitForIOCompletion(INFINITE, this);
            }

            DCHECK(!accept_io_context_.is_pending_ &&
                !read_io_context_.is_pending_ &&
                !write_io_context_.is_pending_);
        }
    }

    bool PipeServer::ClientInfo::Accept()
    {
        if (INVALID_HANDLE_VALUE != pipe_handle_)
        {
            bool ret = ::ConnectNamedPipe(pipe_handle_, &accept_io_context_.overlapped);
            if (ret)
            {
                // Uhm, the API documentation says that this function should never
                // return success when used in overlapped mode.
                NOTREACHED();
                return false;
            }

            DWORD err = ::GetLastError();
            switch (err)
            {
            case ERROR_IO_PENDING:
                accept_io_context_.is_pending_ = true;
                ret = true;
                break;
            case ERROR_PIPE_CONNECTED:
                // 调用ConnectNamedPipe之前就已经连接成功了的client，是否还会走iocp的通知？要验证一下
                //////////////////////////////////////////////////////////////////////////
                ret = true;
                break;
            case ERROR_NO_DATA:
                // The pipe is being closed.
                break;
            default:
                NOTREACHED();
                break;
            }

            return ret;
        }
        return false;
    }

    AsyncType PipeServer::ClientInfo::GetAsyncType(MessagePumpForIO::IOContext* context)
    {
        AsyncType type = AsyncType::Unknown;
        if (&accept_io_context_.overlapped == context)
        {
            type = AsyncType::Pipe_Accept;
        }
        else if (&read_io_context_.overlapped == context)
        {
            type = AsyncType::Pipe_Read;
        }
        else if (&write_io_context_.overlapped == context)
        {
            type = AsyncType::Pipe_Write;
        }
        return type;
    }

    void PipeServer::ClientInfo::OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered, DWORD error)
    {
        auto type = GetAsyncType(context);
        switch (type)
        {
        case AsyncType::Pipe_Accept:
            {
                accept_io_context_.is_pending_ = false;

                // 通知有新的连接到来
                if (pipe_server_)
                {
                    pipe_server_->OnClientConnect(this, error);
                }

                // 在这个通知里面抛送Read异步操作，否则无法持续自动接收数据
                if (error == NOERROR)
                {
                    Read();
                }
            }
            break;
        case AsyncType::Pipe_Read:
            {
                read_io_context_.is_pending_ = false;

                // 通知数据接收完毕
                if (pipe_server_)
                {
                    pipe_server_->OnClientReadData(this, error, read_io_context_.io_buffer.buffer, bytes_transfered);
                }

                // 在这个通知里面抛送Read异步操作，否则无法持续自动接收数据
                if (error == NOERROR)
                {
                    Read();
                }
            }
            break;
        case AsyncType::Pipe_Write:
            {
                write_io_context_.is_pending_ = false;

                if (pipe_server_)
                {
                    pipe_server_->OnClientWriteData(this, error, write_io_context_.io_buffer.buffer, bytes_transfered);
                }
            }
            break;
        default:
            break;
        }

        if (error != NOERROR || type == AsyncType::Unknown)
        {
            if (pipe_handle_ != INVALID_HANDLE_VALUE)
            {
                Close();

                if (pipe_server_)
                {
                    pipe_server_->OnClientError(this, error);
                }
            }
        }
    }


    // PipeClient
    PipeClient::PipeClient(const std::wstring& pipe_name, Delegate* delegate)
        : pipe_name_(pipe_name)
        , delegate_(delegate)
    {
    }

    PipeClient::~PipeClient()
    {
        Close();
    }

    bool PipeClient::Connect()
    {
        MessageLoopForIO* io_message_loop = MessageLoopForIO::current();
        if (!io_message_loop)
        {
            NOTREACHED() << "must be called on exists io thread";
            return false;
        }

        DCHECK(!thread_check_) << "should not called more than once";
        if (!thread_check_)
        {
            thread_check_ = std::make_unique<ThreadChecker>();
        }

        pipe_handle_ = ::CreateFileW(pipe_name_.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION |
            FILE_FLAG_OVERLAPPED,
            NULL);
        if (pipe_handle_ != INVALID_HANDLE_VALUE)
        {
            if (io_message_loop->RegisterIOHandler(pipe_handle_, this))
            {
                if (Read())
                {
                    if (delegate_)
                    {
                        delegate_->OnPipeClientConnect(this, NOERROR);
                    }
                    return true;
                }
            }
        }
        else
        {
            DWORD err = ::GetLastError();
            if (delegate_)
            {
                delegate_->OnPipeClientConnect(this, err);
            }
            /*if (err == ERROR_PIPE_BUSY)
            {
                ::WaitNamedPipeW(pipe_name_.c_str(), 5000);
            }*/
            DLOG(ERROR) << "PipeClient Create failed, code = " << err;
        }
        Close();
        return false;
    }

    bool PipeClient::Send(const char* data, unsigned long len)
    {
        if (thread_check_)
        {
            DCHECK(thread_check_->CalledOnValidThread());
        }

        return Write(data, len);
    }

    void PipeClient::Close()
    {
        if (thread_check_)
        {
            DCHECK(thread_check_->CalledOnValidThread());
        }

        if (pipe_handle_ != INVALID_HANDLE_VALUE)
        {
            if (read_io_context_.is_pending_ ||
                write_io_context_.is_pending_)
            {
                ::CancelIo(pipe_handle_);
            }

            ::DisconnectNamedPipe(pipe_handle_);

            ::CloseHandle(pipe_handle_);
            pipe_handle_ = INVALID_HANDLE_VALUE;

            // 主动循环IOCP，对所有在该pipe上投递的io操作进行处理，
            // 用以确保与pipe相关的资源都能得到处理，这样才能放心进行资源回收/释放管理
            DCHECK(MessageLoopForIO::current());
            while (read_io_context_.is_pending_ || write_io_context_.is_pending_)
            {
                MessageLoopForIO::current()->WaitForIOCompletion(INFINITE, this);
            }

            DCHECK(!read_io_context_.is_pending_ && !write_io_context_.is_pending_);
        }

        thread_check_.reset();
    }

    AsyncType PipeClient::GetAsyncType(MessagePumpForIO::IOContext* context)
    {
        AsyncType type = AsyncType::Unknown;
        if (&read_io_context_.overlapped == context)
        {
            type = AsyncType::Pipe_Read;
        }
        else if (&write_io_context_.overlapped == context)
        {
            type = AsyncType::Pipe_Write;
        }
        return type;
    }

    void PipeClient::OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered, DWORD error)
    {
        auto type = GetAsyncType(context);
        switch (type)
        {
        case AsyncType::Pipe_Read:
            {
                read_io_context_.is_pending_ = false;

                // 通知数据接收完毕
                if (delegate_)
                {
                    delegate_->OnPipeClientReadData(this, error, read_io_context_.io_buffer.buffer, bytes_transfered);
                }

                // 在这个通知里面抛送Read异步操作，否则无法持续自动接收数据
                if (error == NOERROR)
                {
                    Read();
                }
            }
            break;
        case AsyncType::Pipe_Write:
            {
                write_io_context_.is_pending_ = false;

                if (delegate_)
                {
                    delegate_->OnPipeClientWriteData(this, error, write_io_context_.io_buffer.buffer, bytes_transfered);
                }
            }
            break;
        default:
            break;
        }

        if (error != NOERROR || type == AsyncType::Unknown)
        {
            if (pipe_handle_ != INVALID_HANDLE_VALUE)
            {
                Close();

                if (delegate_)
                {
                    delegate_->OnPipeClientError(this, error);
                }
            }
        }
    }

}