#include "thread.h"
#include "thread_util.h"

#include "logging/logging.h"
#include "message_loop/run_loop.h"

namespace
{
    void ThreadFunc(void* params)
    {
        mctm::StdThreadDelegate* thread = static_cast<mctm::StdThreadDelegate*>(params);
        thread->ThreadMain();
    }

    void ThreadQuitHelper()
    {
        mctm::MessageLoop::current()->Quit();
    }
}

namespace mctm
{
    std::unique_ptr<mctm::Thread> Thread::AttachCurrentThread(const char* thread_name, MessageLoop::Type type)
    {
        if (!MessageLoop::current())
        {
            std::unique_ptr<mctm::Thread> thread = std::make_unique<mctm::Thread>(thread_name);
            SetThreadName(::GetCurrentThreadId(), thread_name);
            auto message_loop = std::make_unique<MessageLoop>(type);
            thread->set_message_loop(message_loop.release());
            return std::move(thread);
        }
        return nullptr;
    }

    Thread::Thread(const std::string& thread_name)
        : thread_name_(thread_name)
    {
    }

    Thread::~Thread()
    {
        Stop();
    }

    bool Thread::Start()
    {
        Options options;
        return StartWithOptions(options);
    }

    bool Thread::StartWithOptions(const Options& options)
    {
        startup_data_.options = options;
        thread_ = std::thread(ThreadFunc, this);
        if (thread_.joinable())
        {
            startup_data_.wait_for_run_event.Wait();
            started_ = true;
        }

        return thread_.joinable();
    }
    
    void Thread::Stop()
    {
        if (!started_)
        {
            return;
        }

        // 检查一下当前的消息循环是不是嵌套了多层，如果是嵌了多层的那么Thread::Stop是退不出全部嵌套的，只能退当前的；
        // 如果支持强制退出嵌套循环会带来循环依赖的流程错乱一连串的问题，干脆就不支持了，
        // 想要优雅的退出线程正确的做法应该是Stop前发通知，让消息循环按照自己的逻辑处理业务然后自行退出，以此往上层嵌套递归；
        // 或者直接检测到正处于多层循环嵌套就不支持实际的Stop
        if (message_loop_)
        {
            DCHECK(!message_loop_->IsNested());

            message_loop_->PostTask(FROM_HERE, Bind(ThreadQuitHelper));
        }

        if (thread_.joinable())
        {
            thread_.join();
        }

        message_loop_ = nullptr;
        started_ = false;
    }

    void Thread::set_message_loop(MessageLoop* message_loop)
    {
        message_loop_.reset(message_loop);
    }

    void Thread::ThreadMain()
    {
        SetThreadName(::GetCurrentThreadId(), thread_name_.c_str());

        std::unique_ptr<ScopedCOMInitializer> com;
        if (startup_data_.options.com != NONE)
        {
            com = std::make_unique<ScopedCOMInitializer>(
                startup_data_.options.com == STA ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED);
        }

        message_loop_ = std::make_unique<MessageLoop>(startup_data_.options.type);

        Init();
        
        startup_data_.wait_for_run_event.Signal();

        Run();

        CleanUp();

        message_loop_ = nullptr;
    }

    void Thread::Init()
    {
    }

    void Thread::Run()
    {
        RunLoop run_loop;
        run_loop.Run();
    }

    void Thread::CleanUp()
    {
    }

}