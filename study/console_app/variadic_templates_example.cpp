#include "stdafx.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <iostream>           // std::cout

// http://www.cnblogs.com/qicosmos/p/4309835.html
namespace
{
    //µ›πÈ÷’÷π∫Ø ˝
    void print()
    {
        std::cout << "empty" << std::endl;
    }
    //’πø™∫Ø ˝
    template <class T, class... Args>
    void print(T head, Args... rest)
    {
        std::cout << "parameter " << head << std::endl;
        print(rest...);
    }

    template<size_t N>
    struct Apply
    {
        template<typename F, typename T, typename... A>
        static inline auto apply(F&& f, T&& t, A&&... a)
            -> decltype(Apply<N - 1>::apply(
            std::forward<F>(f), std::forward<T>(t),
            std::get<N - 1>(std::forward<T>(t)),
            std::forward<A>(a)...
            ))
        {
            return Apply<N - 1>::apply(std::forward<F>(f),
                std::forward<T>(t),
                std::get<N - 1>(std::forward<T>(t)),
                std::forward<A>(a)...
                );
        }
    };

    template<>
    struct Apply<0>
    {
        template<typename F, typename T, typename... A>
        static inline auto apply(F&& f, T&&, A&&... a)
            -> decltype(std::forward<F>(f)(std::forward<A>(a)...))
        {
            return std::forward<F>(f)(std::forward<A>(a)...);
        }
    };

    template<typename F, typename T>
    inline auto apply(F&& f, T&& t)
        -> decltype(Apply<std::tuple_size<typename std::decay<T>::type>::value>::
            apply(std::forward<F>(f), std::forward<T>(t)))
    {
        return Apply<std::tuple_size<typename std::decay<T>::type>::value>::
            apply(std::forward<F>(f), std::forward<T>(t));
    }

    void one(int i, double d)
    {
        std::cout << "function one(" << i << ", " << d <<
            ");\n";
    }
    int two(int i)
    {
        std::cout << "function two(" << i << ");\n";
        return i;
    }
}

//≤‚ ‘¥˙¬Î
void variadic_templates_example()
{
    std::tuple<int, double> tup(23, 4.5);
    apply(one, tup);

    int d = apply(two, std::make_tuple(2));
}