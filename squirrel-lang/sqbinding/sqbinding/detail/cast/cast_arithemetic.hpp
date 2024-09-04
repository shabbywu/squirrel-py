#pragma once
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include <squirrel.h>
#include <type_traits>

namespace sqbinding {
namespace detail {
// cast SQObjectPtr/HSQOBJECT to arithemetic
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<FromType>, HSQOBJECT>> * = nullptr,
          typename std::enable_if_t<std::is_arithmetic_v<ToType>> * = nullptr>
ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    if (from._type == tagSQObjectType::OT_INTEGER || from._type == tagSQObjectType::OT_BOOL)
        return _integer(from);
    if (from._type == tagSQObjectType::OT_FLOAT)
        return _float(from);
    throw sqbinding::value_error("unsupported value");
};

// cast signed arithemetic to SQObjectPtr
template <typename FromType, typename ToType, typename std::enable_if_t<std::is_arithmetic_v<FromType>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<ToType, SQObjectPtr>> * = nullptr>
ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    return SQObjectPtr((std::make_signed_t<FromType>)from);
};
} // namespace detail
} // namespace sqbinding
