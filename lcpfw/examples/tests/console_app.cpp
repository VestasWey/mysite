// console_app.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <random>
#include <chrono>

class Pred
{
public:
    Pred()
    {
        static int i = 0;
        id_ = ++i;
        copy_ = false;
        copy_id_ = 0;
    }

    Pred(const Pred &r)/* = delete;*/
    {
        static int i = 0;
        copy_ = true;
        copy_id_ = r.id_;
        id_ = --i;
    }

    Pred(const Pred &&r)
    {
        static int i = 0;
        copy_ = true;
        copy_id_ = r.id_;
        id_ = --i;
    }

    void print() const
    {
        printf("id = %d\n", id_);
    }

    void set_p(int p)
    {
    }

    // 函数对象
    bool operator()()
    {
        return false;
    }

private:

private:
    int id_;
    int copy_id_;
    bool copy_;
};

void vector_study()
{
    std::vector<Pred> vct;
    Pred p, p1, p2;
    vct.push_back(p);
    vct.push_back(p1);
    vct.push_back(p2);
    for (Pred &th : vct)
    {
        th.print();
        th.set_p(1);
    }
    // for each非引用枚举会引起对象拷贝构造，而引用枚举只能用const
    for each (const Pred &var in vct)
    {
        var.print();
        //var.set_p(1);// const枚举不能调用
    }
}

void shuffle_example()
{
    std::vector<std::string> vct{"aa", "bb", "cc", "dd", "ee"};
    unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(vct.begin(), vct.end(), std::default_random_engine(seed));
    for (auto& iter : vct)
    {
        printf("%s, ", iter.c_str());
    }
}

void right_ref_study();
void auto_decltype_study();
void variadic_templates_example();
void thread_std_bind_task_study();
void chromium_post_task_study();
void effective_example();
void thread_message_example();
void dead_lock_example();
void ipv6_example();
void ipc_example();
void mctm_example();
void vector_operation_study();
void try_except_test();
void stl_container_example();
void bus_line_example();
void TestSort();

initializer e_init_val;
extern int g_int;

#define PERFETTO_EINTRW(x)                                   \
  ({                                                        \
    decltype(x) eintr_wrapper_result;                       \
    do {                                                    \
      eintr_wrapper_result = (x);                           \
    } while (eintr_wrapper_result == -1 && errno == EINTR); \
    eintr_wrapper_result;                                   \
  })

#define PERFETTO_EINTR(x)                                       \
  []()->decltype(x)                                             \
  {                                                             \
      decltype(x) eintr_wrapper_result;                         \
      do {                                                      \
        eintr_wrapper_result = (x);                             \
      } while (eintr_wrapper_result == -1 && errno == EINTR);   \
      return eintr_wrapper_result;                              \
  }()

class TestA
{
public:
    void FuncA() {
        printf("TestA::FuncA");
    }
    virtual void FuncB() {
        printf("TestA::FuncA");
    }

private:
    int a;
};

class TestB : public TestA
{
public:
    void FuncA() {
        printf("TestB::FuncA");
    }
    virtual void FuncB() {
        printf("TestB::FuncB");
    }

private:
    int b;
};

int _tmain(int argc, _TCHAR* argv[])
{
    g_int++;
    //size_t bytes_read;
    //bytes_read = PERFETTO_EINTR(::recv(0, nullptr, 0, 0));
    
    std::vector<Pred> vc;
    vc.emplace_back();

    std::cout << std::boolalpha;

    /*TestB* pb = new TestB;
    TestA* pa = pb;
    pa->FuncA();
    pa->FuncB();
    delete pb;

    pb = nullptr;
    pb->FuncA();
    pb->FuncB();*/

    /*TestA a;
    typedef void(TestA::*ClsAMemFn)() ;
    ClsAMemFn pFun = &TestA::FuncA;
    (a.*pFun)();*/

    chromium_post_task_study();
    //vector_study();
    //right_ref_study();
    //auto_decltype_study();
    //variadic_templates_example();
    //thread_atomic_study();
    //thread_post_task_study();
    //effective_example();
    //thread_message_example();
    //dead_lock_example();
    //ipv6_example();
    //ipc_example();
    //thread_std_bind_task_study();
    //mctm_example();
    //shuffle_example();
    //vector_operation_study();
    //try_except_test();
    //stl_container_example();
    //bus_line_example();
    //TestSort();
    system("pause");
	return 0;
}

