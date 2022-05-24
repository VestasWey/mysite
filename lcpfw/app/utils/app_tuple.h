#pragma once

#include <tuple>
#include <type_traits>

// Copy from std::apply(introduced since C++ 17) with only C++ 11/14 features available.
// Function apply(fn, tuple) invokes `fn` and forwards elements in `tuple` as arguments to it.

namespace lcpfw {

#if !_HAS_CXX17

    template <class _Callable, class _Tuple, size_t... _Indices>
    constexpr decltype(auto) _Apply_impl(
        _Callable&& _Obj, _Tuple&& _Tpl, _STD index_sequence<_Indices...>)
    { // invoke _Obj with the elements of _Tpl
        return _STD invoke(_STD forward<_Callable>(_Obj), _STD get<_Indices>(_STD forward<_Tuple>(_Tpl))...);
    }

    template <class _Callable, class _Tuple>
    constexpr decltype(auto) apply(_Callable&& _Obj, _Tuple&& _Tpl)
    { // invoke _Obj with the elements of _Tpl
        return _Apply_impl(_STD forward<_Callable>(_Obj), _STD forward<_Tuple>(_Tpl),
            _STD make_index_sequence<_STD tuple_size_v<_STD remove_reference_t<_Tuple>>>{});
    }
#else
    template <class _Callable, class _Tuple>
    using apply<_Callable, _Tuple> = std::apply<_Callable, _Tuple>;
#endif

}
