#pragma once
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/types/sqtable.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include <type_traits>
namespace sqbinding {
namespace detail {
// cast detail::Table to SQObjectPtr|HSQOBJECT
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, detail::Table>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<ToType>, HSQOBJECT>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    return from.pTable();
}

// cast HSQOBJECT to SQObjectPtr|HSQOBJECT
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<FromType>, HSQOBJECT>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, detail::Table>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    if (from._type == tagSQObjectType::OT_TABLE)
        return detail::Table(_table(from), vm);
    throw sqbinding::value_error("unsupported value");
}

} // namespace detail
} // namespace sqbinding
