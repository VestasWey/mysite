#include "stdafx.h"

#include <iostream>           // std::cout
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
#include <xutility>


namespace
{
    class Foo
    {
    public:
        Foo()
        {
            str_ = "2233";
        }

        Foo(const std::string& str)
        {
            str_ = str;
        }

        std::string cmpstr() { return str_; }
        std::string cmpstr_c() const { return str_; }
        const std::string c_cmpstr() { return str_; }
        const std::string c_cmpstr_c() const { return str_; }

        std::string& r_cmpstr() { return str_; }
        //std::string& r_cmpstr_c() const { return str_; }  // error
        const std::string& cr_cmpstr() { return str_; }
        const std::string& cr_cmpstr_c() const { return str_; }

    private:
        std::string str_;
    };

    std::string g_cmp(Foo* obj)
    {
        std::string str = obj->cmpstr();
        // do sth...
        return str;
    }
}

using namespace std;

namespace
{
#if __cplusplus >= 201103L 
//#if __cplusplus

    template <class _Ty>
    using remove_reference = std::remove_reference<_Ty>;

#else

    /*
    * @remove_reference：copy from C++11 <type_traits>，从各种推导类型中提取其原始类型
    */
    template <class _Ty>
    struct remove_reference {
        typedef _Ty type;
    };

    template <class _Ty>
    struct remove_reference<const _Ty> {
        typedef _Ty type;
    };

    template <class _Ty>
    struct remove_reference<_Ty&> {
        typedef _Ty type;
    };

    template <class _Ty>
    struct remove_reference<const _Ty&> {
        typedef _Ty type;
    };


    // copy from c++11 <xutility>
    template <class _InIt, class _Pr>
    _InIt find_if(_InIt _First, const _InIt _Last, _Pr _Pred) { // find first satisfying _Pred
        for (; _First != _Last; ++_First) {
            if (_Pred(*_First)) {
                break;
            }
        }

        return _First;
    }

#endif

    template <class _Ty, class _Pre>
    bool findif(_Ty* t, _Pre func)
    {
        return func(t);
    }


    /*
    * Comparator、BindComparator 是对Google Chromium base库的Bind、Callback的拙劣模仿，仅仅只是将其作为一个充当比较器角色的函数对象。
    * 
    * @Comparator：一个用于做比较的仿函数模板，接受一个函数指针以及一个被比较的目标值作为构造入参，
                   仿函数执行时通过指定目标类实例来切实调用函数指针所指向的函数，用其返回值与目标值作比较，
                   比较的结果用-1/0/1来返回，类似于strcmp；
    * @BindComparator：旨在于为find_if快速的构造一个比较器函数对象，为调用者隐藏Comparator的细节，精简其使用Comparator、find_if的代码；
    */
    template <class Sig>
    class Comparator;
    
    template <class R, class T>
    class Comparator<R(T::*)()>
    {
        typedef typename remove_reference<R>::type value_type;

    public:
        Comparator(int cmp_type, R(T::* method)(), value_type value)
            : cmp_type_(cmp_type),
            method_(method),
            value_(value)
        {
        }

        bool operator()(T* obj) {
            if (cmp_type_ < 0)
            {
                return (obj->*method_)() < value_;
            }
            else if (cmp_type_ > 0)
            {
                return (obj->*method_)() > value_;
            }

            return (obj->*method_)() == value_;
        }

    private:
        int cmp_type_;
        R(T::* method_)();
        value_type value_;
    };

    template <class R, class T>
    class Comparator<R(T::*)()const>
    {
        typedef typename remove_reference<R>::type value_type;

    public:
        Comparator(int cmp_type, R(T::* method)() const, value_type value)
            : cmp_type_(cmp_type),
            method_(method),
            value_(value)
        {
        }

        bool operator()(T* obj) const {
            if (cmp_type_ < 0)
            {
                return (obj->*method_)() < value_;
            }
            else if (cmp_type_ > 0)
            {
                return (obj->*method_)() > value_;
            }

            return (obj->*method_)() == value_;
        }

    private:
        int cmp_type_;
        R(T::* method_)() const;
        value_type value_;
    };

    template <class R, class T>
    class Comparator<R(*)(T*)>
    {
        typedef typename remove_reference<R>::type value_type;

    public:
        Comparator(int cmp_type, R(*functor)(T*), value_type value)
            : cmp_type_(cmp_type),
            functor_(functor),
            value_(value)
        {
        }

        bool operator()(T* obj) {
            if (cmp_type_ < 0)
            {
                return functor_(obj) < value_;
            }
            else if (cmp_type_ > 0)
            {
                return functor_(obj) > value_;
            }

            return functor_(obj) == value_;
        }

    private:
        int cmp_type_;
        R(*functor_)(T*);
        value_type value_;
    };

    template <class _Vul>
    class Comparator
    {
        typedef typename remove_reference<_Vul>::type value_type;

    public:
        Comparator(int cmp_type, value_type value)
            : cmp_type_(cmp_type),
            value_(value)
        {
        }

        bool operator()(value_type lhz) {
            if (cmp_type_ < 0)
            {
                return lhz < value_;
            }
            else if (cmp_type_ > 0)
            {
                return lhz > value_;
            }

            return lhz == value_;
        }

    private:
        int cmp_type_;
        value_type value_;
    };

    /*
    * 构造等于比较器
    */
    template <class R, class T>
    Comparator<R(T::*)()> BindComparator(R(T::* method)(), typename remove_reference<R>::type vul)
    {
        return Comparator<R(T::*)()>(0, method, vul);
    }

    template <class R, class T>
    Comparator<R(T::*)()const> BindComparator(R(T::* method)()const, typename remove_reference<R>::type vul)
    {
        return Comparator<R(T::*)()const>(0, method, vul);
    }

    template <class R, class T>
    Comparator<R(*)(T*)> BindComparator(R(*method)(T*), typename remove_reference<R>::type vul)
    {
        return Comparator<R(*)(T*)>(0, method, vul);
    }

    template <class R>
    Comparator<typename remove_reference<R>::type> BindComparator(R vul)
    {
        return Comparator<typename remove_reference<R>::type>(0, static_cast<typename remove_reference<R>::type>(vul));
    }

    /*
    * 构造小于比较器
    */
    template <class R, class T>
    Comparator<R(T::*)()> BindComparator_LT(R(T::* method)(), typename remove_reference<R>::type vul)
    {
        return Comparator<R(T::*)()>(-1, method, vul);
    }

    template <class R, class T>
    Comparator<R(T::*)()const> BindComparator_LT(R(T::* method)()const, typename remove_reference<R>::type vul)
    {
        return Comparator<R(T::*)()const>(-1, method, vul);
    }

    template <class R, class T>
    Comparator<R(*)(T*)> BindComparator_LT(R(*method)(T*), typename remove_reference<R>::type vul)
    {
        return Comparator<R(*)(T*)>(-1, method, vul);
    }

    template <class R>
    Comparator<typename remove_reference<R>::type> BindComparator_LT(R vul)
    {
        return Comparator<typename remove_reference<R>::type>(-1, static_cast<typename remove_reference<R>::type>(vul));
    }

    /*
    * 构造大于比较器
    */
    template <class R, class T>
    Comparator<R(T::*)()> BindComparator_GT(R(T::* method)(), typename remove_reference<R>::type vul)
    {
        return Comparator<R(T::*)()>(1, method, vul);
    }

    template <class R, class T>
    Comparator<R(T::*)()const> BindComparator_GT(R(T::* method)()const, typename remove_reference<R>::type vul)
    {
        return Comparator<R(T::*)()const>(1, method, vul);
    }

    template <class R, class T>
    Comparator<R(*)(T*)> BindComparator_GT(R(*method)(T*), typename remove_reference<R>::type vul)
    {
        return Comparator<R(*)(T*)>(1, method, vul);
    }

    template <class R>
    Comparator<typename remove_reference<R>::type> BindComparator_GT(R vul)
    {
        return Comparator<typename remove_reference<R>::type>(1, static_cast<typename remove_reference<R>::type>(vul));
    }

}

void cxx03_example()
{
    printf("__cplusplus=%ld\n", __cplusplus);

    Foo foo;
    bool bb;

    std::string str("2233");

    //std::string cmpstr() { return str_; }
    //std::string cmpstr_c() const { return str_; }
    //const std::string c_cmpstr() { return str_; }
    //const std::string c_cmpstr_c() const { return str_; }
    //
    //std::string& r_cmpstr() { return str_; }
    ////std::string& r_cmpstr_c() const { return str_; }  // error
    //const std::string& cr_cmpstr() { return str_; }
    //const std::string& cr_cmpstr_c() const { return str_; }

    Comparator<std::string(Foo::*)()> cmpstr = BindComparator(&Foo::cmpstr, str);
    bb = cmpstr(&foo);

    Comparator<std::string(Foo::*)() const> cmpstr_c = BindComparator(&Foo::cmpstr_c, str);
    bb = cmpstr_c(&foo);

    Comparator<const std::string(Foo::*)()> c_cmpstr = BindComparator(&Foo::c_cmpstr, str);
    bb = c_cmpstr(&foo);

    Comparator<const std::string(Foo::*)() const> c_cmpstr_c = BindComparator(&Foo::c_cmpstr_c, str);
    bb = c_cmpstr_c(&foo);

    Comparator<std::string&(Foo::*)()> r_cmpstr = BindComparator(&Foo::r_cmpstr, str);
    bb = r_cmpstr(&foo);

    Comparator<const std::string& (Foo::*)() > cr_cmpstr = BindComparator(&Foo::cr_cmpstr, str);
    bb = cr_cmpstr(&foo);

    Comparator<const std::string&(Foo::*)() const> cr_cmpstr_c = BindComparator(&Foo::cr_cmpstr_c, str);
    bb = cr_cmpstr_c(&foo);

    Comparator<std::string(*)(Foo*)> gcmp = BindComparator(g_cmp, str);
    bb = gcmp(&foo);

    bb = findif(&foo, BindComparator(&Foo::cmpstr, str));
    bb = findif(&foo, BindComparator(&Foo::cmpstr_c, "223"));
    bb = findif(&foo, BindComparator(&Foo::c_cmpstr, str));
    bb = findif(&foo, BindComparator(&Foo::c_cmpstr_c, "22233"));

    bb = findif(&foo, BindComparator(&Foo::r_cmpstr, str));
    bb = findif(&foo, BindComparator(&Foo::cr_cmpstr, "223333"));
    bb = findif(&foo, BindComparator(&Foo::cr_cmpstr_c, str));

    std::vector<Foo*> foo_vct{
        new  Foo({"4"}),
        new  Foo({"23"}),
        new  Foo({"101"}),
        /*{"23"},
        {"101"},*/
    };
    str = "101";
    std::vector<Foo*>::iterator foo_iter = find_if(foo_vct.begin(), foo_vct.end(), BindComparator(g_cmp, str));

    std::vector<std::string> str_vct {
        {"4"},
        {"23"},
        {"101"},
    };
    str = "23";
    std::vector<std::string>::iterator str_iter = find_if(str_vct.begin(), str_vct.end(), BindComparator(str));

    return;
}