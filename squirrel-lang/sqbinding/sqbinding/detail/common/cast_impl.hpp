#pragma once
#include <type_traits>
#include <concepts>
#include <variant>
#include <squirrel.h>
#include "format.hpp"
#include "cast_def.hpp"
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

        template <class Return, class...Args>
        class GenericCast<SQObjectPtr(std::function<Return(Args...)>&)> {
            public:
            static SQObjectPtr cast(VM vm, std::function<Return(Args...)>& obj) {
                return detail::CppFunction<Return(Args...)>::Create(obj, vm);
            }
        };

        template <class Return, class...Args>
        class GenericCast<SQObjectPtr(Return(*)(Args...))> {
            public:
            static SQObjectPtr cast(VM vm, Return(*obj)(Args...)) {
                return detail::CppFunction<Return(Args...)>::Create(obj, vm);
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
}
