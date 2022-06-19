#include "semaphore.h"

#include <handleapi.h>
#include <synchapi.h>
#include <WinBase.h>
#include <thread>
#include <iostream>
#include <conio.h>


namespace mctm
{
}

void TestSemaphore()
{
    mctm::Semaphore sem(0, 100);
    bool quit = false;

    std::atomic_int cc = 0;
    std::thread t1([&]()
        {
            std::cout << "thread 1 begin" << std::endl;
            while (!quit)
            {
                sem.Wait();
                std::cout << "thread 1 resume cc=" << cc++ << std::endl;
            }
            std::cout << "thread 1 end" << std::endl;
        });
    std::thread t2([&]()
        {
            std::cout << "thread 2 begin" << std::endl;
            while (!quit)
            {
                sem.Wait();
                std::cout << "thread 2 resume cc=" << cc++ << std::endl;
            }
            std::cout << "thread 2 end" << std::endl;
        });
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            sem.Signal();
        }
        std::this_thread::yield();
    }
    while (!quit)
    {
        int ch = _getch();
        if (ch == VK_ESCAPE)
        {
            std::cout << "quit" << std::endl;
            quit = true;
            sem.Signal();
            sem.Signal();
            break;
        }
    }
    t1.join();
    t2.join();
}