#include "stdafx.h"
#include "LuaProxy.h"
#include <stdio.h>


int CLuaProxy::g_inc = 0;

CLuaProxy::CLuaProxy() 
    : m_id(g_inc++)
    , m_nRef(1)
{
    printf("CLuaProxy construct %d \n", m_id);
}

CLuaProxy::~CLuaProxy()
{
    printf("CLuaProxy destruct %d \n", m_id);
}

void CLuaProxy::SayHello()
{
    printf("CLuaProxy SayHello %d \n", m_id);
}

void CLuaProxy::AddRef()
{
    ++m_nRef;
}

void CLuaProxy::Release()
{
    --m_nRef;

    if (m_nRef <= 0)
    {
        delete this;
    }
}



// test
template <class T>
T MyClass<T>::add(T t1, T t2)
{
    return t1 + t2;
}

template <class T>
T MyClass<T>::static_Add(T t1, T t2)
{
    return t1 * t2;
}

template <class T>
T sub(T t1, T t2)
{
    return t1 - t2;
}

int nor(int t1, int t2)
{
    return t1 * t2;
}

// 强制类模板实例化
//template class MyClass<int>;
//template int MyClass<int>::static_Add(int, int);

// 强制函数模板实例化
//template int sub(int t1, int t2);
