#pragma once
#ifndef SQFUNCTION_ClosureBase
#define SQFUNCTION_ClosureBase
namespace sqbinding { namespace detail {
    class ClosureBase {
        virtual void bindThis(SQObjectPtr &pthis) = 0;
        virtual void bindThis(SQObjectPtr &&pthis) = 0;
    };
}}
#endif


#ifndef SQOBJECTPTR_GETTER_TEMPLATE
#define SQOBJECTPTR_GETTER_TEMPLATE\
    template <typename TK, typename TV>\
    TV get(TK& key, typename std::enable_if_t<std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        SQObjectPtr r;\
        VM& vm = holder->GetVM();\
        if(get(key, r)) {\
            auto v = GenericCast<TV(SQObjectPtr&)>::cast(vm, r);\
            v.bindThis(holder->GetSQObjectPtr());\
            return v;\
        }\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        throw sqbinding::key_error(sqobject_to_string(sqkey) + " does not exists");\
    }\
\
    template <typename TK, typename TV>\
    TV get(TK&& key, typename std::enable_if_t<std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        SQObjectPtr r;\
        VM& vm = holder->GetVM();\
        if(get(key, r)) {\
            auto v = GenericCast<TV(SQObjectPtr&)>::cast(vm, r);\
            v.bindThis(holder->GetSQObjectPtr());\
            return v;\
        }\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        throw sqbinding::key_error(sqobject_to_string(sqkey) + " does not exists");\
    }\
\
    template <typename TK, typename TV>\
    bool get(TK& key, TV& v, typename std::enable_if_t<std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        VM& vm = holder->GetVM();\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        SQObjectPtr ptr;\
        if (!get(sqkey, ptr)) {\
            return false;\
        }\
        v = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);\
        v.bindThis(holder->GetSQObjectPtr());\
        return true;\
    }\
\
    template <typename TK, typename TV>\
    bool get(TK&& key, TV& v, typename std::enable_if_t<std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        VM& vm = holder->GetVM();\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        SQObjectPtr ptr;\
        if (!get(sqkey, ptr)) {\
            return false;\
        }\
        v = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);\
        v.bindThis(holder->GetSQObjectPtr());\
        return true;\
    }\
\
\
    template <typename TK, typename TV>\
    TV get(TK& key, typename std::enable_if_t<!std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        SQObjectPtr r;\
        VM& vm = holder->GetVM();\
        if(get(key, r)) {\
            return GenericCast<TV(SQObjectPtr&)>::cast(vm, r);\
        }\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        throw sqbinding::key_error(sqobject_to_string(sqkey) + " does not exists");\
    }\
\
    template <typename TK, typename TV>\
    TV get(TK&& key, typename std::enable_if_t<!std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        SQObjectPtr r;\
        VM& vm = holder->GetVM();\
        if(get(key, r)) {\
            return GenericCast<TV(SQObjectPtr&)>::cast(vm, r);\
        }\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        throw sqbinding::key_error(sqobject_to_string(sqkey) + " does not exists");\
    }\
\
    template <typename TK, typename TV>\
    bool get(TK& key, TV& r, typename std::enable_if_t<!std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        VM& vm = holder->GetVM();\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        SQObjectPtr ptr;\
        if (!get(sqkey, ptr)) {\
            return false;\
        }\
        r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);\
        return true;\
    }\
\
    template <typename TK, typename TV>\
    bool get(TK&& key, TV& r, typename std::enable_if_t<!std::is_base_of_v<ClosureBase, TV>>* = 0) {\
        VM& vm = holder->GetVM();\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        SQObjectPtr ptr;\
        if (!get(sqkey, ptr)) {\
            return false;\
        }\
        r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);\
        return true;\
    }\

#endif
