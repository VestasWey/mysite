#include "stdafx.h"

#include <conio.h>
#include <memory>

#include "data_encapsulation/smart_pointer.h"
#include "ipc/ipc_channel.h"
#include "logging/logging.h"
#include "message_loop/message_loop.h"
#include "threading/thread.h"

namespace ipc_message
{
    enum ExampleIPCMessageType : unsigned int
    {
        IPC_MSG_BEGIN = 100,

        IPC_S_TO_C,

        IPC_C_TO_S,

        IPC_BOTHWAY,

        IPC_MSG_END,
    };

}

class BusinessIPCChannel;
class BusinessIPCChannelListener
{
public:
    virtual bool OnMessageReceived(BusinessIPCChannel* channel, const mctm::IPCMessage& message) = 0;
    virtual void OnChannelConnected(BusinessIPCChannel* channel, int peer_pid) {}
    virtual void OnChannelError(BusinessIPCChannel* channel) {}

protected:
    virtual ~BusinessIPCChannelListener() = default;
};

class BusinessIPCChannel
    : public mctm::IPCListener
{
public:
    BusinessIPCChannel(const std::wstring& pipe_name, const std::wstring& pipe_instance_mutex_name,
        BusinessIPCChannelListener* listener)
        : pipe_name_(pipe_name)
        , pipe_instance_mutex_name_(pipe_instance_mutex_name)
        , listener_(listener)
    {
    }

    ~BusinessIPCChannel()
    {
        DCHECK(mctm::MessageLoopForIO::current());
        shutdown_ = true;
    }

    bool Init()
    {
        DCHECK(mctm::MessageLoopForIO::current());

        if (!mctm::MessageLoopForIO::current())
        {
            return false;
        }

        instance_mutex_.SetHandle(::CreateMutexW(nullptr, false, pipe_instance_mutex_name_.c_str()));
        if (!instance_mutex_)
        {
            return false;
        }

        server_mode_ = (::GetLastError() != ERROR_ALREADY_EXISTS);
        Connect(server_mode_);

        return true;
    }

    bool Send(mctm::IPCMessage* message)
    {
        DCHECK(mctm::MessageLoopForIO::current());
        if (!channel_)
        {
            return false;
        }

        return channel_->Send(message);
    }

    bool server_mode() const
    {
        return server_mode_;
    }

    void SetListener(BusinessIPCChannelListener* listener)
    {
        std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
        listener_ = listener;
    }

private:
    void Connect(bool server_mode)
    {
        bool ret = false;
        if (server_mode)
        {
            channel_.reset(new mctm::IPCChannel(pipe_name_, mctm::IPCChannel::MODE_SERVER, this));
            ret = channel_->Connect();
        }
        else
        {
            channel_.reset(new mctm::IPCChannel(pipe_name_, mctm::IPCChannel::MODE_CLIENT, this));
            ret = channel_->Connect();
        }
        if (!ret)
        {
            channel_.reset();
            NOTREACHED();
        }
    }

    // Override mctm::IPCListener，invoke on worker thread
    void OnChannelConnected(mctm::IPCChannel* channel, int peer_pid) override
    {
        std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
        if (listener_)
        {
            return listener_->OnChannelConnected(this, peer_pid);
        }
    }

    bool OnMessageReceived(mctm::IPCChannel* channel, const mctm::IPCMessage& message) override
    {
        std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
        if (listener_)
        {
            return listener_->OnMessageReceived(this, message);
        }
        return true;
    }

    void OnChannelError(mctm::IPCChannel* channel) override
    {
        // PipeServer可以设置为客户端断开后自动添加accept实例，
        // 但为了和chromium的IPCChannel封装尽量一致，就不允许IPCChannel其自动增补实例了，
        // 由上层自行在客户端断开通知中进行accept实例的增补
        if (server_mode_ && !shutdown_)
        {
            LOG(INFO) << "Server ipc channel error! Recreat again!";
            Connect(true);
        }

        std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
        if (listener_)
        {
            listener_->OnChannelError(this);
        }
    }

private:
    std::wstring pipe_name_;
    std::wstring pipe_instance_mutex_name_;
    mctm::ScopedHandle instance_mutex_;
    bool server_mode_ = false;
    bool shutdown_ = false;
    std::unique_ptr<mctm::IPCChannel> channel_ = nullptr;

    std::recursive_mutex listener_mutex_;
    BusinessIPCChannelListener* listener_ = nullptr;
};


void chromium_ipc_example()
{
    mctm::Thread::Options option;
    option.type = mctm::MessageLoop::Type::TYPE_IO;
    mctm::Thread thread("chromium_ipc_example_thread");
    thread.StartWithOptions(option);

    std::unique_ptr<BusinessIPCChannel> ipc_channel;

    class IPCChannelListener : public BusinessIPCChannelListener
    {
    public:
        IPCChannelListener() = default;
        virtual bool OnMessageReceived(BusinessIPCChannel* channel, const mctm::IPCMessage& message) override
        {
            switch (message.type())
            {
            case ipc_message::IPC_S_TO_C:
                DLOG(INFO) << "IPC_S_TO_C";
                break;
            case ipc_message::IPC_BOTHWAY:
                DLOG(INFO) << "IPC_BOTHWAY";
                break;
            default:
                break;
            }
            return true;
        }

        virtual void OnChannelConnected(BusinessIPCChannel* channel, int peer_pid) override
        {
        }

        virtual void OnChannelError(BusinessIPCChannel* channel) override
        {
        }

    protected:
    private:
    };
    IPCChannelListener listener;

    int input_ch = 0;
    do
    {
        input_ch = ::_getch();

        switch (input_ch)
        {
        case VK_ESCAPE:
            {
                if (ipc_channel)
                {
                    ipc_channel->SetListener(nullptr);
                    thread.message_loop()->DeleteSoon(FROM_HERE, ipc_channel.release());
                }
            }
            break;
        case 0x31://1
            {
                ipc_channel.reset(new BusinessIPCChannel(
                    LR"(\\.\pipe\chrome.example_ipc_channel)",
                    LR"({242222A3-3016-47A6-9814-64DEE01DC36A})", &listener));
                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&BusinessIPCChannel::Init, ipc_channel.get()));
            }
            break;
        case 0x32://2
            {
                std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                    mctm::MSG_ROUTING_NONE,
                    ipc_message::IPC_C_TO_S,
                    mctm::IPCMessage::PRIORITY_NORMAL);

                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&BusinessIPCChannel::Send, ipc_channel.get(), msg.release()));
            }
            break;
        case 0x33://3
            {
                std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                    mctm::MSG_ROUTING_NONE,
                    ipc_message::IPC_BOTHWAY,
                    mctm::IPCMessage::PRIORITY_NORMAL);

                thread.message_loop()->PostTask(FROM_HERE,
                    mctm::Bind(&BusinessIPCChannel::Send, ipc_channel.get(), msg.release()));
            }
            break;
        default:
            break;
        }

    } while (input_ch != VK_ESCAPE);

    thread.Stop();
}