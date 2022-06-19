#pragma once
#include <atomic>
#include <memory>
#include <windef.h>
#include <WinUser.h>

#include "data_encapsulation/smart_pointer.h"
#include "iocp/iocp.h"
#include "synchronization/waitable_event.h"
#include "time/time_util.h"

namespace mctm
{
    class WaitableEvent;

    class MessagePump
    {
    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            //************************************
            // Method:    ShouldQuitCurrentLoop
            // Returns:   bool，是否退出当前消息循环
            //************************************
            virtual bool ShouldQuitCurrentLoop() = 0;

            //************************************
            // Method:    QuitCurrentLoop
            // Remark:    退出当前消息循环
            //************************************
            virtual void QuitCurrentLoopNow() = 0;

            //************************************
            // Method:    CheckExtensionalLoopSignal
            // Remark:    提供一个时机以便上层触发循环激活信号
            //************************************
            virtual bool CheckExtensionalLoopSignal() = 0;

            //************************************
            // Method:    DoWork、DoDelayedWork、DoIdleWord
            // Returns:   bool，是否还有更多的任务待执行
            //************************************
            virtual bool DoWork() = 0;
            virtual bool DoDelayedWork(TimeTicks* next_delayed_work_time) = 0;
            virtual bool DoIdleWord() = 0;
        };

        explicit MessagePump(Delegate* delegate);
        virtual ~MessagePump() = default;

        //************************************
        // Method:    DoRunLoop
        // FullName:  mctm::MessagePump::DoRunLoop
        // Access:    virtual public 
        // Returns:   void
        // Remark:    无限循环的主函数
        //************************************
        virtual void DoRunLoop() = 0;

        //************************************
        // Method:    ScheduleWork
        // FullName:  mctm::MessagePump::ScheduleWork
        // Access:    virtual public 
        // Returns:   void
        // Remark:    新任务到来之后通知循环结束等待，快速进行新一轮循环以便及时处理新任务
        //************************************
        virtual void ScheduleWork() = 0;

        //************************************
        // Method:    ScheduleDelayedWork
        // FullName:  mctm::MessagePump::ScheduleDelayedWork
        // Access:    virtual public 
        // Returns:   void
        // Parameter: const TimeTicks & delayed_work_time
        // Remark:    通知循环泵以指定的时间节点（delayed_work_time）进行循环信号等待
        //************************************
        virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time) = 0;

    protected:
        int GetCurrentDelay() const;

    protected:
        Delegate* delegate_ = nullptr;
        TimeTicks delayed_work_time_;
        std::atomic_bool have_work_ = false;
    };

    // 以Event为信号点进行循环的泵
    class MessagePumpDefault : public MessagePump
    {
    public:
        explicit MessagePumpDefault(Delegate* delegate);
        virtual ~MessagePumpDefault();

    protected:
        // MessagePump
        void DoRunLoop() override;
        void ScheduleWork() override;
        void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    private:
        void WaitForWork();

    private:
        WaitableEvent event_;
    };

    // 以系统消息（GetMessage/PeekMessage）为信号点进行循环的泵
    class MessagePumpForUI : public MessagePump
    {
    public:
        class MessageFilter
        {
        public:
            virtual ~MessageFilter() = default;

            virtual bool DoPeekMessage(LPMSG lpMsg,
                HWND hWnd,
                UINT wMsgFilterMin,
                UINT wMsgFilterMax,
                UINT wRemoveMsg);
            virtual bool ProcessMessage(const MSG& msg);
        };
        
        explicit MessagePumpForUI(MessagePump::Delegate* delegate);
        virtual ~MessagePumpForUI();

    protected:
        // MessagePump
        void DoRunLoop() override;
        void ScheduleWork() override;
        void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    private:
        static LRESULT CALLBACK WndProcThunk(
            HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
        void InitMessageWnd();
        void WaitForWork();
        bool ProcessNextWindowsMessage();
        bool ProcessMessageHelper(const MSG& msg);
        void WillProcessMessage(const MSG& msg);
        void DidProcessMessage(const MSG& msg);
        bool ProcessPumpScheduleWorkMessage();
        void HandleWorkMessage();
        void HandleTimerMessage();

    private:
        std::shared_ptr<MessageFilter> message_filter_;
        ATOM atom_ = 0;
        HWND message_hwnd_ = nullptr;
    };

    // 以I/O完成端口（IOCP）为信号点进行循环的泵
    class MessagePumpForIO : public MessagePump
    {
    public:
        using IOContext = OVERLAPPED;

        class IOHandler
        {
        public:
            virtual ~IOHandler() {}

            virtual void OnIOCompleted(IOContext* context, DWORD bytes_transfered,
                DWORD error) = 0;
        };

        explicit MessagePumpForIO(MessagePump::Delegate* delegate);

        bool RegisterIOHandler(HANDLE file_handle, IOHandler* handler);
        bool RegisterJobObject(HANDLE job_handle, IOHandler* handler);
        bool WaitForIOCompletion(DWORD timeout, IOHandler* filter);

    protected:
        // MessagePump
        void DoRunLoop() override;
        void ScheduleWork() override;
        void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    private:
        void WaitForWork();
        bool ProcessInternalIOItem(const IOCP::IOItem& item);
        void WillProcessIOEvent();
        void DidProcessIOEvent();

    private:
        IOCP iocp_;
    };
}

