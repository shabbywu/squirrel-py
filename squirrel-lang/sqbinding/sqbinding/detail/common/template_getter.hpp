#pragma once
#ifndef SQOBJECTPTR_GETTER_TEMPLATE
#define SQOBJECTPTR_GETTER_TEMPLATE\
    template <typename TK, typename TV>\
    TV get(TK& key) {\
        TV r;\
        if(get(key, r)) {\
            return r;\
        }\
        VM& vm = holder->GetVM();\
        auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);\
        throw sqbinding::key_error(sqobject_to_string(sqkey));\
    }\
\
    template <typename TK, typename TV>\
    bool get(TK& key, TV& r) {\
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
