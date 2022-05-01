// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include <stdio.h>
#include <tchar.h>



// TODO:  在此处引用程序需要的其他头文件

#include <iostream>

class initializer
{
public:
    initializer()
    {
        if (s_counter_++ == 0) init();
    }

    ~initializer()
    {
        if (--s_counter_ == 0) clean();
    }

    void print()
    {
        printf("s_counter_=%d \n", s_counter_);
    }

    int s_counter_ = 0;

private:
    void init() {}
    void clean() {}
};
