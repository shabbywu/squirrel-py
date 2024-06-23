#pragma once
#include <concepts>
#include <variant>
#include <squirrel.h>
#include "format.hpp"
#include "cast_def.hpp"
#include "errors.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/detail/types/cppfunction.hpp"


typedef std::variant<SQInteger, SQFloat> SafeSQType;


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

    namespace detail {
        template <>
        class GenericCast<SQObjectPtr(SQInteger&)> {
            public:
            static SQObjectPtr cast(VM vm, SQInteger& obj) {return SQObjectPtr(obj);}
        };

        template <>
        class GenericCast<SQObjectPtr(SQInteger&&)> {
            public:
            static SQObjectPtr cast(VM vm, SQInteger&& obj) {return SQObjectPtr(obj);}
        };

        template <>
        class GenericCast<SQObjectPtr(std::string&)> {
            public:
            static SQObjectPtr cast(VM vm, std::string& obj) {return SQObjectPtr(SQString::Create(_ss(*vm), obj.c_str(), obj.size()));}
        };

        template <>
        class GenericCast<SQObjectPtr(std::string&&)> {
            public:
            static SQObjectPtr cast(VM vm, std::string&& obj) {return SQObjectPtr(SQString::Create(_ss(*vm), obj.c_str(), obj.size()));}
        };

        // template <class Return, class...Args>
        // class GenericCast<SQObjectPtr(std::function<Return(Args...)>&)> {
        //     public:
        //     static SQObjectPtr cast(VM vm, std::function<Return(Args...)>& obj) {
        //         return make_stack_object<detail::cpp_function>(vm, obj).second;
        //     }
        // };

        // template <class Return, class...Args>
        // class GenericCast<SQObjectPtr(Return(*)(Args...))> {
        //     public:
        //     static SQObjectPtr cast(VM vm, Return(*obj)(Args...)) {
        //         return make_stack_object<detail::cpp_function>(vm, obj).second;
        //     }
        // };
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

        template <class ToType>
        class GenericCast<ToType(HSQOBJECT&)> {
            public:
            static ToType cast(VM vm, HSQOBJECT& obj) {
                switch (obj._type) {
                    // case tagSQObjectType::OT_NULL:
                    //     return GenericCast<void, ToType>::cast(vm, obj);
                    case tagSQObjectType::OT_INTEGER:
                        return GenericCast<ToType(SQInteger)>::cast(vm, _integer(obj));
                    case tagSQObjectType::OT_FLOAT:
                        return GenericCast<ToType(SQFloat)>::cast(vm, _float(obj));
                    case tagSQObjectType::OT_BOOL:
                        return GenericCast<ToType(SQBool)>::cast(vm, _integer(obj));
                    // case tagSQObjectType::OT_STRING:
                    //     return GenericCast<SQChar*, ToType>::cast(vm, _stringval(obj));
                    // case tagSQObjectType::OT_TABLE:
                    //     return GenericCast<SQTable*, ToType>::cast(vm, _table(obj));
                    // case tagSQObjectType::OT_ARRAY:
                    //     return GenericCast<SQArray*, ToType>::cast(vm, _array(obj));
                    // case tagSQObjectType::OT_USERDATA:
                    //     return GenericCast<SQUserData*, ToType>::cast(vm, _userdataval(obj));
                    // case tagSQObjectType::OT_CLOSURE:
                    //     return GenericCast<SQClosure*, ToType>::cast(vm, _closure(obj));
                    // case tagSQObjectType::OT_NATIVECLOSURE:
                    //     return GenericCast<SQNativeClosure*, ToType>::cast(vm, _nativeclosure(obj));
                    // case tagSQObjectType::OT_GENERATOR:
                    //     return GenericCast<SQGenerator*, ToType>::cast(vm, _generator(obj));
                    // case tagSQObjectType::OT_USERPOINTER:
                    //     return GenericCast<SQUserPointer, ToType>::cast(vm, _userpointer(obj));
                    // case tagSQObjectType::OT_THREAD:
                    //     return GenericCast<SQVM*, ToType>::cast(vm, _thread(obj));
                    // case tagSQObjectType::OT_FUNCPROTO:
                    //     return GenericCast<SQFunctionProto*, ToType>::cast(vm, _funcproto(obj));
                    // case tagSQObjectType::OT_CLASS:
                    //     return GenericCast<SQClass*, ToType>::cast(vm, _class(obj));
                    // case tagSQObjectType::OT_INSTANCE:
                    //     return GenericCast<SQInstance*, ToType>::cast(vm, _instance(obj));
                    // case tagSQObjectType::OT_WEAKREF:
                    //     return GenericCast<SQWeakRef*, ToType>::cast(vm, _weakref(obj));
                    // case tagSQObjectType::OT_OUTER:
                    //     return GenericCast<SQOuter*, ToType>::cast(vm, _outer(obj));
                    default:
                        throw sqbinding::value_error("unsupported value");
                }
            }
        };

        template <class FromType>
        class GenericCast<void(FromType&)> {
            public:
            static void cast(VM vm, FromType& obj) {}
        };

        template <class FromType>
        class GenericCast<void(FromType&&)> {
            public:
            static void cast(VM vm, FromType&& obj) {}
        };

    }

    namespace detail {
        template <class ToType>
        class GenericCast<ToType(SQInteger)> {
            public:
            static ToType cast(VM vm, SQInteger obj) {
                return (ToType)obj;
            }
        };

        template <class ToType>
        class GenericCast<ToType(SQFloat)> {
            public:
            static ToType cast(VM vm, SQFloat obj) {
                return (ToType)obj;
            }
        };

        template <class ToType>
        class GenericCast<ToType(SQBool)> {
            public:
            static ToType cast(VM vm, SQBool obj) {
                return (ToType)obj;
            }
        };
    }
}
