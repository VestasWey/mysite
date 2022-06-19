#include "stdafx.h"
#include <string>

#include "def.h"


namespace
{
    class BaseClass
    {
    public:
        explicit BaseClass(const char* str)
            : str_(str)
        {
        }

        virtual ~BaseClass() = default;

        void print() const
        {
            printf("%s \n", str_.c_str());
        }

        std::string* operator->()
        {
            return &str_;
        }

        std::string& operator*()
        {
            return str_;
        }

        // 隐式转换操作符
        operator std::string()
        {
            return str_;
        }

        virtual void fly() = 0
        {
            printf("BaseClass::fly() \n");
        }
        virtual void fly(int dst)
        {
            printf("fly to dst \n");
        }

    private:
        std::string str_;
    };

    void test_BaseClass_func(const BaseClass &base)
    {
        base.print();
    }

    void test_string_func(const std::string &str)
    {
        printf("%s \n", str.c_str());
    }

    class DervClass : public BaseClass
    {
    public:
        using BaseClass::fly;

        explicit DervClass(const char* str)
            : BaseClass(str)
        {
        }

        virtual void fly() override
        {
            BaseClass::fly();

            printf("DervClass::fly() \n");
        }

    };

    class Mem
    {
    public:
        Mem()
        {
            mm = 1;
        }
        ~Mem()
        {
        }

        int mm;
    };

    class Mem1
    {
    public:
        Mem1()
        {
            mm = 1;
        }
        ~Mem1()
        {
        }

        int mm;
    };

    class Base
    {
    public:
        Base()
        {
            func1();
        }

        virtual ~Base()
        {
            func1();
        }

        virtual void func1() {
            printf("Base::func1 \n");
        }

        virtual void func2() {
            printf("Base::func2 \n");
        }

        Mem bb;
    };

    class Base1
    {
    public:
        Base1()
        {
        }

        ~Base1()
        {

        }

        Mem1 bb1;
    };

    class Dev : public Base1, public Base
    {
    public:
        Dev()
        {
            func1();
        }

        ~Dev()
        {
            func1();
        }

        virtual void func1() {
            printf("Dev::func1 \n");
        }

        Mem1 m1;
        Mem m;
    };
    
    class CtrlModel {
    public:

        virtual void func() = 0;
        virtual ~CtrlModel() = default;
    };
    class CtrlView {
    public:
        CtrlView(CtrlModel* model)
            : model_(model) {}

        ~CtrlView()
        {
            model_->func();	// _purecall
        }
    private:
        CtrlModel* model_;
    };
    class View : public CtrlModel
    {
    public:
        View()
        {
            child_.reset(new CtrlView(this));
        }
        ~View()
        {
        }

        void func() override {}
    private:
        std::unique_ptr<CtrlView> child_;
    };

    //间接基类A
    class A {
    public:
        virtual void print() {
            printf("A::print\n");
        }

        void printNoVir() {
            printf("A::printNoVir\n");
        }
    protected:
        int m_a;
        int m_aa;
    };
    //直接基类B
    class B : virtual public A {  //虚继承
    public:
        void print() override {
            printf("B::print\n");
        }

        virtual void pout() {
            printf("B::pout\n");
        }

        void printNoVir() {
            printf("B::printNoVir\n");
        }
    protected:
        int m_a;
        int m_b = 22;
    };
    //直接基类C
    class C : virtual public A {  //虚继承
    public:
        void print() override {
            printf("C::print\n");
        }

        virtual void pout() {
            printf("C::pout\n");
        }

        void printNoVir() {
            printf("C::printNoVir\n");
        }
    protected:
        int m_a;
        int m_c = 33;
    };
    //派生类D
    class D : public B, public C {
    public:
        D()
        {
            B::m_a = 1;
            C::m_a = 2;
            m_aa = 1122;
            A::m_a = -3;
        }
        void seta(int a) {
            B::m_a = a;
        }  //正确
        void setb(int b) {
            m_b = b;
        }  //正确
        void setc(int c) {
            m_c = c;
        }  //正确
        void setd(int d) {
            m_d = d;
        }  //正确

        void print() override {
            printf("D::print\n");
        }

        /*void printNoVir() {
            printf("D::printNoVir\n");
        }*/
    private:
        int m_d;
        //int m_a = -1;
    };
}

const int c_int = 1;
static int s_int = 1;
//int g_int = 1;

extern Mem gm;
extern Mem1 gm1;

Mem1 gm1;
Mem gm;

void effective_example()
{
    /*p_init_val.s_counter_--;
    printf("effective_example print p_init_val\n");
    p_init_val.print();
    s_init_val.s_counter_--;
    printf("effective_example print s_init_val\n");
    s_init_val.print();
    printf("effective_example print c_init_val address %p \n", &c_init_val);
    printf("effective_example print e_init_val address %p \n", &e_init_val);*/

    //test_BaseClass_func("effect"); // error, BaseClass's contruct with "explicit" flag

    //DervClass basec("test_string_func");
    //basec.fly();
    //basec.fly(1);
    //basec.print();
    //test_string_func(basec); // operator std::string()
    //printf("%s \n", basec->c_str()); // std::string* operator->()

    /*int ss = sizeof(Mem);
    Dev* dev = new Dev;
    Mem* bb = &dev->bb;
    Mem1* bb1 = &dev->bb1;
    Mem* m = &dev->m;
    Mem1* m1 = &dev->m1;
    delete dev;*/

    /*View* view = new View();
    delete view;*/

    // 一、普通菱形继承
    // 二、一个虚继承、一个普通继承
    //   1、成员变量的使用
    //      D类也有基类A同名变量的，直接通过D引用的话就是D类的变量；
    //      D类保有基类A的两份数据，派生类D要通过显式指定间接类型B/C的方式引用A的变量，如：B::m_a = 1;、C::m_a = 2;
    // 
    //   2、普通成员函数的使用
    //      D类也有基类A同名函数的，子类D的函数优先级最高，直接通过D调用的就是D类版本的函数；
    //      D类没有基类A同名函数的，B/C类其中之一有的，则通过D调用的是B/C类的函数；
    //      D类没有基类A同名函数的，B/C类二者都有的，则产生“对函数的访问不明确”的编译错误；
    //      可以显式的指定类型调用指定类型的函数，如：d.A::printNoVir();、d.B::printNoVir();、d.C::printNoVir();
    //
    //   3、虚函数的使用
    //      D类重写类A的虚函数的，直接通过D引用的话就是D类重写的版本；
    //      D类没有重写A的虚函数，B/C类其中之一重写了A的虚函数，则通过D调用的话用的是B/C类重写的版本;
    //      D类没有重写A的虚函数，B/C类二者都重写了A的虚函数，则产生“对函数的访问不明确”的编译错误；
    //      可以显式的指定类型调用指定类型的函数，如：d.A::print();、d.B::print();、d.C::print();

    // 三、两个虚继承
    //  1、成员变量的使用
    //      D类也有基类A同名变量的，直接通过D引用的话就是D类的变量；
    //      D类只保有基类A的一份数据，即使派生类D通过显式指定间接类型B/C的方式引用A的变量，引用到的都是同一个地址，如：B::m_a = 1;、C::m_a = 2;，最终m_a=2;
    // 
    // 总结
    // 不管是双虚继承 还是 双普通继承 还是 一个普通继承一个虚继承，
    // 针对函数（不管是虚函数重写还是普通函数覆盖），有如下3种共性：
    //    1、派生类有跟基类重名的函数的，派生类的优先级最高，通过派生类调用的函数都是派生类自己的版本；
    //    2、派生类没有跟基类重名的函数的，但是中间类有的话分两种情况：
    //       a.多个中间类都有，会编译失败，其中根据失败类型可以分为继承关系不明确和函数调用不明确：
    //          ① 如果是多个中间类都重写了基类虚函数，则派生类的声明处会产生“派生类对基类的不明确继承”的编译错误；
    //          ② 如果是多个中间类都有同名普通函数，那么通过派生类进行函数的调用处会产生“对函数的访问不明确”的编译错误；
    //       b.只有一个中间类中有，那么调用的就是这个中间类的函数版本；
    //    3、都可以通过显式指定中间类型调用中间类型的函数版本，如 d.A::print();、d.B::print();、d.C::print()；
    // 
    // 针对成员变量：
    //    1、派生类有跟基类同名变量的，派生类的优先级最高，直接通过派生类引用的就是派生类自己的变量，且派生类的成员和基类的成员是互相独立的变量；
    //    2、派生类没有跟基类同名变量的，但是中间类有的话，中间类的变量和基类的变量也是互相独立的变量，要引用这些变量需要显式指定类型，如：B::m_a = 1;、A::m_a = 2;；
    //    3、派生类和中间类都没有跟基类同名的变量的，有两种情况：
    //       a.“中间类中只有一个类是虚继承基类另一个是普通继承基类”和“两个中间类都是普通继承基类”的情况一样，
    //         即派生类还是会保有互相独立的两份基类数据，派生类可以通过显式指定中间类型来引用这两份基类数据，如：B::m_a = 1;、C::m_a = 2;
    //       b.所有中间类都是虚继承基类，则派生类只保有一份基类数据，即使派生类通过显式指定间接类型的方式引用基类的变量，引用到的都是同一个地址，如：B::m_a = 1;、C::m_a = 2;，最终m_a=2;

    D d;
    d.seta('c');
    d.A::print();
    d.B::print();
    d.C::print();

    d.A::printNoVir();
    d.B::printNoVir();
    d.C::printNoVir();

    //d.pout();
    d.print();
    //d.printNoVir();

}