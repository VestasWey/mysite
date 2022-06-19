#include "stdafx.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <iostream>           // std::cout

// http://www.cnblogs.com/qicosmos/p/4309835.html
namespace
{
    //µÝ¹éÖÕÖ¹º¯Êý
    void print()
    {
        std::cout << "empty" << std::endl;
    }
    //Õ¹¿ªº¯Êý
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

    // enable_if
    template <typename T>
    struct is_smart_pointer_helper : public std::false_type {};

    template <typename T>
    struct is_smart_pointer_helper<std::shared_ptr<T>> : public std::true_type {};

    template <typename T>
    struct is_smart_pointer_helper<std::unique_ptr<T>> : public std::true_type {};

    template <typename T>
    struct is_smart_pointer_helper<std::weak_ptr<T>> : public std::true_type {};

    template <typename T>
    struct is_smart_pointer : public is_smart_pointer_helper<typename std::remove_cv<T>::type> {};

    template <typename T>
    typename std::enable_if<is_smart_pointer<T>::value, void>::type check_smart_pointer(const T& t)
    {
        std::cout << "is smart pointer" << std::endl;
    }

    template <typename T>
    typename std::enable_if<!is_smart_pointer<T>::value, void>::type check_smart_pointer(const T& t)
    {
        std::cout << "not smart pointer" << std::endl;
    }

    // chromium
#define TRACE_VALUE_TYPE_UINT (static_cast<unsigned char>(2))
#define TRACE_VALUE_TYPE_INT (static_cast<unsigned char>(3))

    template <typename T, class = void>
    struct TraceValueHelper {};

    // TraceValue::Helper for integers and enums.
    template <typename T>
    struct TraceValueHelper<
        T,
        typename std::enable_if<std::is_integral<T>::value ||
        std::is_enum<T>::value>::type> {
        static constexpr unsigned char kType =
            std::is_signed<T>::value ? TRACE_VALUE_TYPE_INT : TRACE_VALUE_TYPE_UINT;
        /*static inline void SetValue(TraceValue* v, T value) {
            v->as_uint = static_cast<unsigned long long>(value);
        }*/
    };

    template <typename T>
    struct HasHelperSupport {
    private:
        using Yes = char[1];
        using No = char[2];

        template <typename V>
        static Yes& check_support(
            /*decltype(TraceValueHelper<typename InnerType<V>::type>::kType,
                int()));*/
            decltype(TraceValueHelper<V>::kType,
                int()));
        template <typename V>
        static No& check_support(...);

    public:
        static constexpr bool value = sizeof(Yes) == sizeof(check_support<T>(0));
    };

    template <typename T>
    struct InnerType {
        using type = typename std::remove_cv<typename std::remove_reference<
            typename std::decay<T>::type>::type>::type;
    };

    template <typename T,
        class = std::enable_if_t<
        HasHelperSupport<typename InnerType<T>::type>::value>>
    struct TypeCheck {
        static const bool value = true;
    };
}

//²âÊÔ´úÂë
void variadic_templates_example()
{
    auto tt = std::is_integral<int>::value;
    auto et = std::is_enum<int>::value;
    auto hp = TraceValueHelper<int>::kType;
    auto hhp = HasHelperSupport<int>::value;
    auto cp = TypeCheck<int>::value;
    InnerType<int>::type it = 22;
    /*std::tuple<int, double> tup(23, 4.5);
    apply(one, tup);
    int d = apply(two, std::make_tuple(2));*/

    int* p(new int(2));
    std::shared_ptr<int> pp(new int(2));
    std::unique_ptr<int> upp(new int(4));
    check_smart_pointer(p);
    check_smart_pointer(pp);
    check_smart_pointer(upp);
}