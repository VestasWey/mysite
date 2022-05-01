#pragma once

namespace mctm
{
    class MessageLoop;
    class RunLoop
    {
    public:
        RunLoop();
        ~RunLoop();

        void Run();
        void Quit();
        bool quitted() const;
        bool running() const;

    private:
        bool BeforeRun();
        void AfterRun();

    private:
        friend class MessageLoop;

        MessageLoop* message_loop_;
        RunLoop* previous_run_loop_ = nullptr;
        bool quit_called_ = false;
        bool running_ = false;
        int run_depth_ = 1;
        bool quit_when_idle_received_ = false;
    };
}

