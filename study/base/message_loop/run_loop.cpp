#include "run_loop.h"
#include "message_loop.h"


namespace mctm
{
    RunLoop::RunLoop()
        : message_loop_(MessageLoop::current())
    {
    }

    RunLoop::~RunLoop()
    {
    }

    void RunLoop::Run()
    {
        if (message_loop_)
        {
            if (!BeforeRun())
            {
                return;
            }
            message_loop_->DoRunLoop();
            AfterRun();
        }
    }

    void RunLoop::Quit()
    {
        quit_called_ = true;
    }

    bool RunLoop::quitted() const
    {
        return quit_called_;
    }

    bool RunLoop::running() const
    {
        return running_;
    }

    bool RunLoop::BeforeRun()
    {
        if (quit_called_ || running_)
        {
            return false;
        }

        RunLoop* current_run_loop = message_loop_->current_run_loop();
        if (current_run_loop && current_run_loop->quitted())
        {
            return false;
        }

        previous_run_loop_ = current_run_loop;
        message_loop_->set_run_loop(this);
        run_depth_ = previous_run_loop_ ? previous_run_loop_->run_depth_ + 1 : 1;

        running_ = true;
        return true;
    }

    void RunLoop::AfterRun()
    {
        running_ = false;
        message_loop_->set_run_loop(previous_run_loop_);
        message_loop_ = nullptr;
    }

}
