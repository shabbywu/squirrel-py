#pragma once
#include <concepts>
#include <variant>
#include <string>
#include <squirrel.h>
#include "format.hpp"
#include "cast_def.hpp"
#include "errors.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"
#include "sqbinding/detail/types/sqvm.hpp"


typedef std::variant<SQInteger, SQFloat, bool> SafeSQType;

namespace sqbinding {
    namespace detail {
        static std::string sqobject_to_string(SQObjectPtr& self) {
            switch (self._type) {
                case tagSQObjectType::OT_NULL:
                    return std::string("OT_NULL");
                case tagSQObjectType::OT_INTEGER:
                    return string_format("OT_INTEGER: {%d}", _integer(self));
                case tagSQObjectType::OT_FLOAT:
                    return string_format("OT_FLOAT: {%f}", _float(self));
                case tagSQObjectType::OT_BOOL:
                    return string_format("OT_BOOL: {%s}", _integer(self)?"true":"false");
                case tagSQObjectType::OT_STRING:
                    return string_format("OT_STRING: {%s}", _stringval(self));
                case tagSQObjectType::OT_TABLE:
                    return string_format("OT_TABLE: {%p}[{%p}]", _table(self), _table(self)->_delegate);
                case tagSQObjectType::OT_ARRAY:
                    return string_format("OT_ARRAY: {%p}", _array(self));
                case tagSQObjectType::OT_USERDATA:
                    return string_format("OT_USERDATA: {%p}[{%p}]", _userdataval(self),_userdata(self)->_delegate);
                case tagSQObjectType::OT_CLOSURE:
                    return string_format("OT_CLOSURE: [{%p}]", _closure(self));
                case tagSQObjectType::OT_NATIVECLOSURE:
                    return string_format("OT_NATIVECLOSURE: [{%p}]", _nativeclosure(self));
                case tagSQObjectType::OT_GENERATOR:
                    return string_format("OT_GENERATOR: [{%p}]", _generator(self));
                case tagSQObjectType::OT_USERPOINTER:
                    return string_format("OT_USERPOINTER: [{%p}]", _userpointer(self));
                case tagSQObjectType::OT_THREAD:
                    return string_format("OT_THREAD: [{%p}]", _thread(self));
                case tagSQObjectType::OT_FUNCPROTO:
                    return string_format("OT_FUNCPROTO: [{%p}]", _funcproto(self));
                case tagSQObjectType::OT_CLASS:
                    return string_format("OT_CLASS: [{%p}]", _class(self));
                case tagSQObjectType::OT_INSTANCE:
                    return string_format("OT_INSTANCE: [{%p}]", _instance(self));
                case tagSQObjectType::OT_WEAKREF:
                    return string_format("OT_WEAKREF: [{%p}]", _weakref(self));
                case tagSQObjectType::OT_OUTER:
                    return string_format("OT_OUTER: [{%p}]", _outer(self));
                default:
                    return string_format("TYPE_UNKNOWN: [{%p}]", &self);
            }
        }
    }

    // cast any to SQObjectPtr
    namespace detail {
        // generic case
        template<typename Type>
        class GenericCast<SQObjectPtr(Type&), typename std::enable_if_t<std::is_convertible_v<Type, SafeSQType>>> {
            public:
            static SQObjectPtr cast(VM vm, Type& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast " << typeid(Type&).name() << " to SQObjectPtr" << std::endl;
                #endif
                return SQObjectPtr(obj);
            }
        };

        // cast std::string
        template <>
        class GenericCast<SQObjectPtr(std::string&)> {
            public:
            static SQObjectPtr cast(VM vm, std::string& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast std::string& to SQObjectPtr" << std::endl;
                #endif
                return SQObjectPtr(SQString::Create(_ss(*vm), obj.c_str(), obj.size()));
            }
        };

        template <>
        class GenericCast<SQObjectPtr(std::string&&)> {
            public:
            static SQObjectPtr cast(VM vm, std::string&& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast std::string&& to SQObjectPtr" << std::endl;
                #endif
                return SQObjectPtr(SQString::Create(_ss(*vm), obj.c_str(), obj.size()));
            }
        };

        template <>
        class GenericCast<SQObjectPtr(cpp_function&)> {
            public:
            static SQObjectPtr cast(VM vm, cpp_function& func) {
                    SQUserPointer ptr = sq_newuserdata(*vm, sizeof(cpp_function));
                    std::memcpy(ptr, &func, sizeof(cpp_function));

                    SQRELEASEHOOK hook = [](SQUserPointer ptr, SQInteger)->SQInteger {
                        #ifdef TRACE_CONTAINER_GC
                        std::cout << "GC::Release " << typeid(cpp_function).name() << std::endl;
                        #endif
                        return 0;
                    };
                    sq_setreleasehook(*vm, -1, hook);
                    // get userdata in stack top
                    SQUserData* ud = _userdata((*vm)->PopGet());
                    return ud;
            }
        };
    }

    namespace detail {
        template <class FromType>
        class GenericCast<void(FromType&)> {
            public:
            static void cast(VM vm, FromType& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast " << typeid(FromType&).name() <<" to None" << std::endl;
                #endif
            }
        };

        template <class FromType>
        class GenericCast<void(FromType&&)> {
            public:
            static void cast(VM vm, FromType&& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast " << typeid(FromType&&).name() <<" to None" << std::endl;
                #endif
            }
        };
    }

    // cast SQObject to Any
    namespace detail {
        // cast SQInteger
        template<typename Integer>
        class GenericCast<Integer(HSQOBJECT&), typename std::enable_if_t<std::is_integral_v<Integer>>> {
            public:
            static Integer cast(VM vm, HSQOBJECT& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast SQInteger to" << typeid(Integer).name() << std::endl;
                #endif
                if (obj._type == tagSQObjectType::OT_INTEGER) return _integer(obj);
                throw sqbinding::value_error("unsupported value");
            }
        };

        template<typename Integer>
        class GenericCast<Integer(SQObjectPtr&), typename std::enable_if_t<std::is_integral_v<Integer>>> {
            public:
            static Integer cast(VM vm, SQObjectPtr& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast SQInteger to" << typeid(Integer).name() << std::endl;
                #endif
                if (obj._type == tagSQObjectType::OT_INTEGER) return _integer(obj);
                throw sqbinding::value_error("unsupported value");
            }
        };

        // cast SQFloat
        template<typename Floating>
        class GenericCast<Floating(HSQOBJECT&), typename std::enable_if_t<std::is_floating_point_v<Floating>>> {
            public:
            static Floating cast(VM vm, HSQOBJECT& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast SQFloat to" << typeid(Floating).name() << std::endl;
                #endif
                if (obj._type == tagSQObjectType::OT_FLOAT) return _float(obj);
                throw sqbinding::value_error("unsupported value");
            }
        };

        template<>
        class GenericCast<bool(HSQOBJECT&)> {
            public:
            static bool cast(VM vm, HSQOBJECT& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast SQBool to bool" << std::endl;
                #endif
                if (obj._type == tagSQObjectType::OT_BOOL) return _integer(obj);
                throw sqbinding::value_error("unsupported value");
            }
        };

        template<>
        class GenericCast<std::string(HSQOBJECT&)> {
            public:
            static std::string cast(VM vm, HSQOBJECT& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast HSQOBJECT to std::string" << std::endl;
                #endif
                if (obj._type == tagSQObjectType::OT_STRING) return _stringval(obj);
                throw sqbinding::value_error("unsupported value");
            }
        };
    }
}
