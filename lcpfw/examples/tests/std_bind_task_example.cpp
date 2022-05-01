#include "stdafx.h"

#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <windows.h>
#include <conio.h>
#include <vector>
#include <atomic>
#include <sstream>
#include <memory>
#include <queue>
#include <map>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include "functional/callback.h"


/*
 * 
 */

using namespace std::placeholders;

// https://www.cnblogs.com/qicosmos/p/4325949.html 可变模版参数
namespace
{
    class TestBindCls
    {
    public:
        int Run(const std::string& str)
        {
            std::cout << "Run -> '" << str << "'" << std::endl;
            return -2233;
        }

        int RunConst(const std::string& str) const
        {
            std::cout << "RunConst -> '" << str << "'" << std::endl;
            return 2233;
        }

        static TestBindCls CreateObject(const std::string& str)
        {
            std::cout << "CreateObject -> '" << str << "'" << std::endl;
            return TestBindCls();
        }
    };

    int GlobalRun(const std::string& str)
    {
        std::cout << "GlobalRun -> '" << str << "'" << std::endl;
        return -3454;
    }

    template<typename R>
    auto InvokeLambda(const std::function<R()>& fn) -> R
    {
        return fn();
    }
}


void thread_std_bind_task_study()
{
    std::shared_ptr<TestBindCls> sp = std::make_shared<TestBindCls>();
    std::weak_ptr<TestBindCls> wp(sp);

    /*auto sfn(std::bind(&TestBindCls::Run, sp, _1));
    auto bwp = std::bind(&TestBindCls::Run, wp, _1);
    auto rwp = std::bind(&TestBindCls::Run, sp.get(), _1);
    sfn("sfn");
    bwp("bwp");
    rwp("rwp");*/

    auto nfn = mctm::Bind(&TestBindCls::Run, wp, "wp");
    auto ret = nfn.Run();

    mctm::Closure cfn(mctm::Bind(&TestBindCls::RunConst, sp, "sp"));
    cfn.Run();

    auto rnfn = mctm::Bind(&TestBindCls::Run, sp.get(), "raw");
    auto rnret = rnfn.Run();

    auto sfn = mctm::Bind(TestBindCls::CreateObject, "CreateObject");
    auto obj = sfn.Run();

    auto gfn = mctm::Bind(GlobalRun, "GlobalRun");
    auto gret = gfn.Run();

    auto ld  = std::bind([](const std::string& str) {}, "GlobalRun");

    /*auto ldfn = mctm::Bind(InvokeLambda<>, ld);
    auto ldret = ldfn.Run();*/

    mctm::Closure cgfn(mctm::Bind(GlobalRun, "Closure"));
    mctm::Closure cc = cgfn;
    mctm::Closure cl(cgfn);
    cc.Run();
    cl.Run();

    return;
}