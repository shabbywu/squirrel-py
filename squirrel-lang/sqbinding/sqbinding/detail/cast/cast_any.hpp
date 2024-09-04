#pragma once
#include "sqbinding/detail/types/sqvm.hpp"
#include <type_traits>
namespace sqbinding {
namespace detail {
// cast anything to itself
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, std::decay_t<ToType>>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    return from;
}

// cast anything to void
template <typename FromType, typename ToType, typename std::enable_if_t<std::is_void_v<ToType>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
}

} // namespace detail
} // namespace sqbinding
