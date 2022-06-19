#pragma once

#include <string>
#include <thread>

#include "data_encapsulation/smart_pointer.h"
#include "synchronization/waitable_event.h"

#include "message_loop/message_loop.h"

/*  Thread类本身不直接接受task投递，需通过其MesssageLoop来做这些工作，采用此结构的原因是：
 *  1、Thread实例并不是在其所封装的线程中被创建的，那么线程与Thread实例的对应关联就必须延后
 *     放到Thread实例本身封装的线程的执行方法ThreadMain中（这个时候才可以通过线程ID等唯一性标识属性
 *     将线程和Thread实例关联起来），
 *  
 *
 *
 *
 **/

namespace mctm
{
    class StdThreadDelegate
    {
    public:
        virtual ~StdThreadDelegate() = default;

        virtual void ThreadMain() = 0;
    };

    class Thread : protected StdThreadDelegate
    {
    public:
        enum ComStatus
        {
            NONE,
            STA,
            MTA,
        };

        struct Options 
        {
            MessageLoop::Type type = MessageLoop::Type::TYPE_DEFAULT;
            ComStatus com = ComStatus::NONE;
        };

        static std::unique_ptr<Thread> AttachCurrentThread(const char* thread_name, MessageLoop::Type type);

        explicit Thread(const std::string& thread_name);
        virtual ~Thread();

        bool Start();
        bool StartWithOptions(const Options& options);
        void Stop();

        MessageLoop* message_loop() const { return message_loop_.get(); }
        MessageLoopRef message_loop_ref() const { return message_loop_; }

    protected:
        void set_message_loop(MessageLoop* message_loop);

        // StdThreadDelegate
        void ThreadMain() override;

        virtual void Init();
        virtual void Run();
        virtual void CleanUp();

    private:
        struct StartupData
        {
            StartupData()
                : wait_for_run_event(false, false)
            {
            }
            
            Options options;
            WaitableEvent wait_for_run_event;
        };

    private:
        std::string thread_name_;
        StartupData startup_data_;
        bool started_ = false;
        std::thread thread_;
        MessageLoopRef message_loop_;
    };
}

