#pragma once
#include <memory>
#include <assert.h>
#include <iostream>

namespace
{
    template<class _Ty>
    struct release_delete
    {	// default deleter for unique_ptr
        typedef release_delete<_Ty> _Myt;

        release_delete()
        {	// default construct
        }

        template<class _Ty2,
        class = typename enable_if<is_convertible<_Ty2 *, _Ty *>::value,
            void>::type>
            release_delete(const release_delete<_Ty2>&)
        {	// construct from another default_delete
        }

        void operator()(_Ty *_Ptr) const
        {	// delete a pointer
            static_assert(0 < sizeof(_Ty),
            "can't release an incomplete type");
            _Ptr->Release();
        }
    };
}

template<class T, class D = std::default_delete<T>, class R = release_delete<T>>
class LuaUserdataWrapper
{
public:
    LuaUserdataWrapper()
        : data(nullptr)
        , deleter_type_(0)
    {
        init();
    }

    LuaUserdataWrapper(T* d, int deleter_type = 0)
        : data(d)
        , deleter_type_(deleter_type)
    {
        init();
    }

    ~LuaUserdataWrapper()
    {
        if (data)
        {
            if (0 == deleter_type_)
            {
                deleter_(data);
            }
            else
            {
                releaser_(data);
            }
        }
    }

    T* get() const { return data; }
    operator T*() const { return data; }
    T* operator->() const
    {
        assert(data != NULL);
        return data;
    }

private:
    void init()
    {
        if (0 != deleter_type_)
        {
            data->AddRef();
        }
    }

private:
    T* data;

    int deleter_type_;
    D deleter_;
    R releaser_;
};

class __declspec(dllexport) CLuaProxy
{
public:
    CLuaProxy();

    ~CLuaProxy();

    void SayHello();

    int ct() const { return m_id; }

    void AddRef();
    void Release();

private:
    static int g_inc;
    int m_id;
    int m_nRef;
};
typedef LuaUserdataWrapper<CLuaProxy> ProxyWrapper;


// test

// 模板类
template <class T>
class MyClass
{
public:
    T add(T t1, T t2);

    static T static_Add(T t1, T t2);
};
// 外部模板
extern template class MyClass<int>;
// 当需要导出时，需要特例化需要导出的类型
template class __declspec(dllexport) MyClass<int>;

// 模板函数
template <class T>
T sub(T t1, T t2);
// 外部模板
extern template int sub(int t1, int t2);
// 模板函数的导出
template __declspec(dllexport) int sub(int t1, int t2);

// 静态函数模板
template < typename T>
static void static_func(T t)
{
    std::cout << "static func :: " << t << std::endl;
};

// 普通函数导出
__declspec(dllexport) int nor(int t1, int t2);