#include "stdafx.h"

#include <conio.h>
#include <windows.h>

#include "ipc/ipc_channel.h"
#include "logging/logging.h"
#include "message_loop/message_loop.h"
#include "message_loop/run_loop.h"
#include "net/pipe/pipe.h"
#include "net/url_request/url_request_context.h"
#include "threading/thread.h"
#include "threading/thread_util.h"
#include "net/url_request/url_fetcher.h"
#include "strings/string_util.h"
#include "third_party/base/md5.h"
#include "synchronization/array.h"
#include "synchronization/semaphore.h"

namespace
{
    int GlobalFunction(const std::string& str)
    {
        std::cout << "GlobalFunction: " << str.c_str() << std::endl;
        return 111;
    }

    void ReplyGlobalFunction()
    {
        std::cout << "ReplyGlobalFunction" << std::endl;
    }

    mctm::Thread thread("mctm_def_thread");

    void TestPipe()
    {
        mctm::Thread::Options option;
        option.type = mctm::MessageLoop::Type::TYPE_IO;
        thread.StartWithOptions(option);

        mctm::PipeServer srv(L"\\\\.\\pipe\\chrome", nullptr, 1);
        mctm::PipeClient cli(L"\\\\.\\pipe\\chrome", nullptr);

        int input_ch = 0;
        do
        {
            input_ch = ::_getch();

            switch (input_ch)
            {
            case VK_ESCAPE:
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::PipeServer::Stop, &srv));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::PipeClient::Close, &cli));

                    //main_thread->message_loop()->Quit();
                }
                break;
            case 0x31://1
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::PipeServer::Start, &srv));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::PipeClient::Connect, &cli));
                }
                break;
            case 0x32://2
                {
                    static char data[] = "mctm::PipeServer::Send";
                    static ULONG_PTR client_key = 0;
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::PipeServer::Send, &srv, client_key, data, strlen(data)));
                }
                break;
            case 0x33://3
                {
                    static char data[] = "mctm::PipeClient::Send";
                    ULONG_PTR client_key = 0;
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::PipeClient::Send, &cli, data, strlen(data)));
                }
                break;
            case 0x34://4
                {
                    thread.message_loop()->PostTaskAndReply(FROM_HERE,
                        mctm::Bind(&GlobalFunction, "PostTaskAndReply"),
                        mctm::Bind(&ReplyGlobalFunction));
                }
                break;
            default:
                {
                }
                break;
            }

        } while (input_ch != VK_ESCAPE);

        thread.Stop();
    }

    void TestIPC()
    {
        enum
        {
            IPC_MSG_REQ = 1001,
            IPC_MSG_RSP = 1002,
        };

        mctm::Thread::Options option;
        option.type = mctm::MessageLoop::Type::TYPE_IO;
        thread.StartWithOptions(option);

        std::unique_ptr<mctm::IPCChannel> srv = std::make_unique<mctm::IPCChannel>
            (L"\\\\.\\pipe\\chrome", mctm::IPCChannel::MODE_SERVER, nullptr);
        std::unique_ptr<mctm::IPCChannel> clt = std::make_unique<mctm::IPCChannel>
            (L"\\\\.\\pipe\\chrome", mctm::IPCChannel::MODE_CLIENT, nullptr);

        int input_ch = 0;
        do
        {
            input_ch = ::_getch();

            switch (input_ch)
            {
            case VK_ESCAPE:
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Close, srv.get()));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Close, clt.get()));
                    thread.message_loop()->DeleteSoon(FROM_HERE, srv.release());
                    thread.message_loop()->DeleteSoon(FROM_HERE, clt.release());

                    //main_thread->message_loop()->Quit();
                }
                break;
            case 0x31://1
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Connect, srv.get()));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Connect, clt.get()));
                }
                break;
            case 0x32://2
                {
                    std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                        mctm::MSG_ROUTING_NONE,
                        IPC_MSG_REQ,
                        mctm::IPCMessage::PRIORITY_NORMAL);

                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Send, srv.get(), msg.release()));
                }
                break;
            case 0x33://3
                {
                    std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                        mctm::MSG_ROUTING_NONE,
                        IPC_MSG_RSP,
                        mctm::IPCMessage::PRIORITY_NORMAL);

                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Send, clt.get(), msg.release()));
                }
                break;
            default:
                break;
            }

        } while (input_ch != VK_ESCAPE);

        thread.Stop();
    }

    void TestURLRequest()
    {
        mctm::Thread::Options option;
        option.type = mctm::MessageLoop::Type::TYPE_IO;
        thread.StartWithOptions(option);

        class TestFetcher : public mctm::URLFetcherDelegate
        {
        public:
        protected:
            void OnURLFetchDownloadData(const mctm::URLFetcher* source,
                const char *ptr, size_t size) override
            {
                data_.append(ptr, size);
                std::wstring wstr = mctm::UTF8ToWide(data_);
                base::MD5Digest hash;
                base::MD5Sum(ptr, size, &hash);
                std::string str = base::MD5DigestToBase16(hash);
                DLOG(INFO) << data_;
            }

        private:
            std::string data_;
        };
        TestFetcher fetcher;

        mctm::URLRequestContext url_context;
        std::shared_ptr<mctm::URLFetcher> url_fetcher;

        auto fn = [&]()->bool
        {
            bool key_pressed = !!::_kbhit();
            if (!key_pressed)
            {
                ::Sleep(1);
                return true;
            }

            int input_ch = ::_getch();
            switch (input_ch)
            {
            case VK_ESCAPE:
                {
                    if (url_fetcher)
                    {
                        url_fetcher->Stop();
                        url_fetcher = nullptr;
                    }
                    mctm::MessageLoop::current()->set_check_extensional_loop_signal_handler(nullptr);
                    mctm::MessageLoop::current()->Quit();
                }
                break;
            case 0x31://1
                {
                    url_fetcher = mctm::URLFetcher::Create(mctm::CanonURL("https://api.live.bilibili.com/xlive/app-blink/v1/entrance/GetEntranceList?access_key=99805d73f14624d4f865873fc78d46a1&appkey=aae92bc66f3edfab&platform=pc_link&sign=8097d2cc3dbafaa5f5f686538c80ac28&ts=1570861672&uid=35274621&version=3.10.0.0"),
                        mctm::URLFetcher::RequestType::GET, &fetcher);
                    url_fetcher->SetRequestContext(&url_context);
                    url_fetcher->SetNetworkTaskRunner(thread.message_loop_ref());
                    url_fetcher->Start();
                }
                break;
            default:
                break;
            }

            return true;
        };

        mctm::MessageLoop::current()->set_check_extensional_loop_signal_handler(fn);
        mctm::RunLoop run_loop;
        run_loop.Run();

        thread.Stop();
    }

    void TestChromiumIPC()
    {
        enum
        {
            IPC_MSG_REQ = 1001,
            IPC_MSG_RSP = 1002,
        };

        mctm::Thread::Options option;
        option.type = mctm::MessageLoop::Type::TYPE_IO;
        thread.StartWithOptions(option);

        std::unique_ptr<mctm::IPCChannel> srv = std::make_unique<mctm::IPCChannel>
            (L"\\\\.\\pipe\\chrome.ipc_channel", mctm::IPCChannel::MODE_SERVER, nullptr);
        std::unique_ptr<mctm::IPCChannel> clt = std::make_unique<mctm::IPCChannel>
            (L"\\\\.\\pipe\\chrome.ipc_channel", mctm::IPCChannel::MODE_CLIENT, nullptr);

        int input_ch = 0;
        do
        {
            input_ch = ::_getch();

            switch (input_ch)
            {
            case VK_ESCAPE:
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Close, srv.get()));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Close, clt.get()));
                    thread.message_loop()->DeleteSoon(FROM_HERE, srv.release());
                    thread.message_loop()->DeleteSoon(FROM_HERE, clt.release());

                    //main_thread->message_loop()->Quit();
                }
                break;
            case 0x31://1
                {
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Connect, srv.get()));
                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Connect, clt.get()));
                }
                break;
            case 0x32://2
                {
                    std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                        mctm::MSG_ROUTING_NONE,
                        IPC_MSG_REQ,
                        mctm::IPCMessage::PRIORITY_NORMAL);

                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Send, srv.get(), msg.release()));
                }
                break;
            case 0x33://3
                {
                    std::unique_ptr<mctm::IPCMessage> msg = std::make_unique<mctm::IPCMessage>(
                        mctm::MSG_ROUTING_NONE,
                        IPC_MSG_RSP,
                        mctm::IPCMessage::PRIORITY_NORMAL);

                    thread.message_loop()->PostTask(FROM_HERE,
                        mctm::Bind(&mctm::IPCChannel::Send, clt.get(), msg.release()));
                }
                break;
            default:
                break;
            }

        } while (input_ch != VK_ESCAPE);

        thread.Stop();
    }
}

void mctm_example()
{
    DLOG(ERROR) << "mctm_example";
    //std::unique_ptr<mctm::Thread> main_thread = mctm::Thread::AttachCurrentThread("main_mctm_thread", mctm::MessageLoop::Type::TYPE_UI);

    //TestPipe();
    //TestIPC();
    //TestURLRequest();
    //TestChromiumIPC();

    //TestCycleArray();
    TestSemaphore();

    /*if (main_thread)
    {
        mctm::RunLoop run_loop;
        run_loop.Run();
    }*/
}