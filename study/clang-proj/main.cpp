// console_app.cpp : 定义控制台应用程序的入口点。
//

//#include <windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <random>
#include <chrono>

#include "base/at_exit.h"
#include "base/command_line.h"


int main(int argc, char* argv[])
{
    base::AtExitManager exit_manager;
    base::CommandLine::Init(0, nullptr);

    //base::CommandLine::ForCurrentProcess()->InitFromArgv(argc, (const wchar_t* const*)argv);
    auto process_type = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII("type");
    //auto qw = base::CommandLine::ForCurrentProcess()->GetSwitchValuePath("type");
    auto qwe = base::CommandLine::ForCurrentProcess()->GetArgs();

    base::CommandLine::Reset();
    system("pause");/// .
    return 0;
}

