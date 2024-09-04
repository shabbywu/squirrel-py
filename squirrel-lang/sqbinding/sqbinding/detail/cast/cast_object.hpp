#pragma once
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/detail/types/sqclass.hpp"
#include <type_traits>
namespace sqbinding {
namespace detail {
// cast SQObjectPtr/HSQOBJECT to pointer
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<FromType>, HSQOBJECT>> * = nullptr,
          typename std::enable_if_t<std::is_class_v<std::remove_pointer_t<ToType>> && std::is_pointer_v<ToType>> * =
              nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;

#endif
    if (from._type == tagSQObjectType::OT_USERDATA) {
        auto holder = (StackObjectHolder<std::remove_pointer_t<ToType>> *)_userdataval(from);
        return &(holder->GetInstance());
    }
    if (from._type == tagSQObjectType::OT_INSTANCE) {
        return (ToType)_instance(from)->_userpointer;
    }
    throw sqbinding::value_error("unsupported value");
}

// cast pointer to SQObjectPtr/HSQOBJECT
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_class_v<std::remove_pointer_t<FromType>> && std::is_pointer_v<FromType>> * =
              nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<ToType>, HSQOBJECT>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    auto clazz = ClassRegistry::getInstance(vm)->find_class_object<std::remove_pointer_t<FromType>>();
    if (clazz != nullptr) {
        auto pClass = clazz->pClass();
        auto instance = pClass->CreateInstance();
        instance->_userpointer = from;
        return instance;
    }
    // fallback
    return SQObjectPtr(detail::make_userdata(vm, std::forward<FromType>(from)));
}

} // namespace detail
} // namespace sqbinding
