#include "array.h"
#include <future>
#include <thread>
#include <chrono>


void TestCycleArray()
{
    int is[95] = { 0 };
    for (int i  = 0; i < 95; i++)
    {
        is[i] = i;
    }
    mctm::CycleArray<int> ary(10);
    ary.write(is, 95);

    /*auto one = ary.read();

    auto rs = ary.read(40);*/

    auto fea0 = std::async(std::launch::async, [&](int len) {

            auto rs = ary.read(len);
            return rs;
        }, 
        40);

    auto fea1 = std::async(std::launch::async, [&]() {

        auto rs = ary.read();
        return rs;
        });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //auto fr0 = fea0.get();
    //auto fr1 = fea1.get();
    system("pause");
}