#pragma once
#include "sqbinding/detail/types/sqvm.hpp"
#include <type_traits>
namespace sqbinding {
namespace detail {
// cast SQObjectPtr to HSQOBJECT
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, SQObjectPtr>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, HSQOBJECT>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    return from;
}

// cast HSQOBJECT to SQObjectPtr
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, HSQOBJECT>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, SQObjectPtr>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    return from;
}

} // namespace detail
} // namespace sqbinding
