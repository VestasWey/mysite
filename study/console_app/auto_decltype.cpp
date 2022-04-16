
#include "stdafx.h"
#include <string>

namespace
{
    class CA
    {
    public:
    protected:
    private:
    };

    int ret_int_func(int) { return 101; }
    int(*func(const std::string&))(int) { return ret_int_func; }

    int needvoid() { return -1; }
    int(*retnedvid())() { return needvoid; }
    int(*(*pf())()) () { return retnedvid; }
    auto pf1()->auto(*)()->int(*)() { return nullptr; }

    int func_ca(const CA&) { return 10; }
    int(*int_func_string(const std::string&))(const CA&) { return func_ca; }
    int(*(*func0(const std::wstring&))(const std::string&)) (const CA&) { return int_func_string; }
    auto func_auto0(const std::wstring&)->auto(*)(const std::string&)->int(*)(const CA&) { return int_func_string; }
    auto func_auto1(const std::wstring&)->auto(*)(const std::string&)->decltype(int_func_string(std::string())) { return int_func_string; }
}

void auto_decltype_study()
{
    auto i = pf();
    auto iret = i();
    auto liret = iret();

    auto j = pf1();
    auto s = func("asd");

    auto f = func0(L"asd");
    auto f0 = func_auto0(L"asd");
    auto f1 = func_auto1(L"asd");
    auto ret = f("func0");
    auto ret0 = f0("func_auto0");
    auto ret1 = f1("func_auto1");
    auto inter = ret(CA());
    auto inter0 = ret0(CA());
    auto inter1 = ret1(CA());
}