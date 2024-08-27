#pragma once
#include "cast_def.hpp"
#include "cast_impl.hpp"
#include "errors.hpp"
#include "format.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/types/sqclass.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include <concepts>
#include <squirrel.h>
#include <string>
#include <variant>

namespace sqbinding {
namespace detail {
template <class T> class GenericCast<T *(HSQOBJECT &), typename std::enable_if_t<std::is_class_v<T>>> {
  public:
    static T *cast(VM vm, HSQOBJECT &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast HSQOBJECT to " << typeid(T *).name() << std::endl;
#endif
        if (obj._type == tagSQObjectType::OT_USERDATA) {
            auto holder = (StackObjectHolder<T> *)_userdataval(obj);
            return &(holder->GetInstance());
        }
        if (obj._type == tagSQObjectType::OT_INSTANCE) {
            return (T *)_instance(obj)->_userpointer;
        }
        throw sqbinding::value_error("unsupported value");
    }
};

template <class T> class GenericCast<T *(SQObjectPtr &), typename std::enable_if_t<std::is_class_v<T>>> {
  public:
    static T *cast(VM vm, SQObjectPtr &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(T *).name() << std::endl;
#endif
        if (obj._type == tagSQObjectType::OT_USERDATA) {
            auto holder = (StackObjectHolder<T> *)_userdataval(obj);
            return &(holder->GetInstance());
        }
        if (obj._type == tagSQObjectType::OT_INSTANCE) {
            return (T *)_instance(obj)->_userpointer;
        }
        throw sqbinding::value_error("unsupported value");
    }
};

template <class T> class GenericCast<SQObjectPtr(T *&), typename std::enable_if_t<std::is_class_v<T>>> {
  public:
    static SQObjectPtr cast(VM vm, T *&obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast " << typeid(T *).name() << " to SQObjectPtr" << std::endl;
#endif
        auto clazz = ClassRegistry::getInstance(vm)->find_class_object<T>();
        if (clazz != nullptr) {
            auto pClass = clazz->pClass();
            auto instance = pClass->CreateInstance();
            instance->_userpointer = obj;
            return instance;
        }
        // fallback
        return SQObjectPtr(detail::make_userdata(vm, std::forward<T *>(obj)));
    }
};

template <class T> class GenericCast<SQObjectPtr(T *), typename std::enable_if_t<std::is_class_v<T>>> {
  public:
    static SQObjectPtr cast(VM vm, T *obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast " << typeid(T *).name() << " to SQObjectPtr" << std::endl;
#endif
        auto clazz = ClassRegistry::getInstance(vm)->find_class_object<T>();
        if (clazz != nullptr) {
            auto pClass = clazz->pClass();
            auto instance = pClass->CreateInstance();
            instance->_userpointer = obj;
            return instance;
        }
        // fallback
        return SQObjectPtr(detail::make_userdata(vm, std::forward<T *>(obj)));
    }
};

} // namespace detail
} // namespace sqbinding
