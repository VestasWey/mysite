#include "app_single_instance_guarantor.h"

#include <windows.h>

//#include "base/ext/bind_lambda.h"
#include "base/strings/utf_string_conversions.h"

//#include "ui/bililive_commands.h"
//#include "ui/bililive_obs.h"

#include "public/main/app_process.h"
//#include "bililive/livehime/pluggable/pluggable_controller.h"
#include "common/app_context.h"
#include "public/main/app_command_ids.h"
#include "public/main/app_thread.h"
#include "public/main/app_process.h"
#include "public/common/app_constants.h"
#include "base/command_line.h"
#include "base/task/current_thread.h"
#include "base/threading/thread_restrictions.h"
#include "base/files/file_util.h"


namespace {

    const wchar_t kInstanceMutexLiveHime[] = L"{9FD4ED93-D289-45CA-AED7-4BCF99C7483C}";
    const wchar_t kMsgActivateStrLiveHime[] = L"{FA99D2D4-BA27-4A92-95CC-A8AD1F0FCFD5}";
    const wchar_t kMsgTransmitCmdline[] = L"{46418A59-EA18-4B6E-8B6F-FDEC49C5D6A9}";

    const wchar_t* mutex_name = kInstanceMutexLiveHime;

    std::map<UINT, DWORD> g_reg_msg_respons;

}   // namespace

AppProcessSingleton::AppProcessSingleton()
{
}

AppProcessSingleton::~AppProcessSingleton()
{}

bool AppProcessSingleton::Install()
{
    msg_activate_id_ = ::RegisterWindowMessageW(kMsgActivateStrLiveHime);
    msg_transmit_cmdline_id_ = ::RegisterWindowMessageW(kMsgTransmitCmdline);
    if (msg_activate_id_ == 0 || 
        msg_transmit_cmdline_id_ == 0)
    {
        PLOG(WARNING) << "Register activate window message failure!";
        return false;
    }

    g_reg_msg_respons.insert({ msg_activate_id_, 0 });
    g_reg_msg_respons.insert({ msg_transmit_cmdline_id_, 0 });

    instance_mutex_.Set(::CreateMutexW(nullptr, false, mutex_name));
    if (!instance_mutex_.IsValid()) {
        PLOG(WARNING) << "Failed to create instance mutex!";
        return false;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        PostMessageW(HWND_BROADCAST, msg_activate_id_, 0, 0);
        return false;
    }

    return true;
}

void AppProcessSingleton::UnInstall()
{
    if (monitoring_) {
        base::CurrentUIThread::Get()->RemoveMessagePumpObserver(this);
    }

    instance_mutex_.Close();
}

void AppProcessSingleton::StartMonitor()
{
    DCHECK(!monitoring_);

    base::CurrentUIThread::Get()->AddMessagePumpObserver(this);
    monitoring_ = true;
}

void AppProcessSingleton::TransmitCommandLine()
{
    /*auto process_type = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(lcpfw::kSwitchProcessType);
    if (process_type == bililive::kProcessScheme)
    {
        std::string args = base::UTF16ToUTF8(base::CommandLine::ForCurrentProcess()->GetCommandLineString());

        LOG(INFO) << "Transmit cmdline to main app process, \n\t" << args;

        base::ThreadRestrictions::ScopedAllowIO allow;
        base::FilePath path;
        if (base::GetTempDir(&path))
        {
            path = path.Append(bililive::kCmdlineTempFileName);
            if (base::WriteFile(path, args.data(), args.length()) == (int)args.length())
            {
                PostMessageW(HWND_BROADCAST, msg_transmit_cmdline_id_, 0, 0);
            }
        }
    }*/
}

// static
void AppProcessSingleton::ActivateMainWindow()
{
    //auto main_window = GetAppProcess()->();
    //if (!main_window) {
    //    return;
    //}
    //
    ////auto weak_bound = main_window->AsWeakPtr();
    //AppThread::PostTask(
    //    AppThread::UI,
    //    FROM_HERE,
    //    base::BindLambda([main_window] {
    //        if (main_window) {
    //            LOG(INFO) << "single instance activate";
    //            //bililive::ExecuteCommand(main_window, IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
    //        }
    //    }));
}

void AppProcessSingleton::WillDispatchMSG(const MSG& msg)
{
    // We may receive several same messages from different windows, filter them out.
    static const DWORD kRefractoryPeriod = 1000;
    if (g_reg_msg_respons.count(msg.message) != 0)
    {
        if ((msg.time - g_reg_msg_respons[msg.message]) > kRefractoryPeriod)
        {
            if (msg.message == msg_activate_id_)
            {
                ActivateMainWindow();
            }
            else if (msg.message == msg_transmit_cmdline_id_)
            {
                //PluggableController::GetInstance()->ScheduleTransmitCmdline();
            }

            g_reg_msg_respons[msg.message] = msg.time;
        }
    }
}

void AppProcessSingleton::DidDispatchMSG(const MSG& msg)
{}
