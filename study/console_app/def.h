#pragma once


// 用static或const定义的变量，是私有变量，这些变量的作用域只在定义的源文件中
static initializer s_init_val;
const initializer c_init_val;

// 放在匿名空间里定义的变量，也是私有变量，作用域只在各自源文件中
namespace {
    initializer p_init_val;
}

// 没有加static或const修饰定义的变量，是全局变量，可以被其他源文件共同使用。当被多个源文件包含时，会在链接阶段出现重复定义的情况（找到一个或多个多重定义的符号），链接阶段失败
//initializer g_init_val;

// extern修饰声明的全局变量，只需要在一个源文件中定义一次，所有源文件都使用的同一个变量
extern initializer e_init_val;