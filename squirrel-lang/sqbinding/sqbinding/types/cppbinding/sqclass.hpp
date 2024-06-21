#pragma once

#include "sqbinding/common/format.h"
#include "definition.h"

namespace sqbinding {
    namespace detail {
        class Class: public std::enable_shared_from_this<Class> {
            public:
                struct Holder {
                    Holder(::SQClass* pClass, HSQUIRRELVM vm) : vm(vm) {
                        clazz = pClass;
                        sq_addref(vm, &clazz);
                    }
                    ~Holder(){
                        sq_release(vm, &clazz);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr clazz;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                Class (::SQClass* pClass, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pClass, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pClass() -> _uiRef;
                }

                ::SQClass* pClass() {
                    return _class(holder->clazz);
                }

            public:
                template <typename TK, typename TV>
                void set(TK& key, TV& val) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    auto sqval = generic_cast<std::remove_reference_t<TV>, SQObjectPtr>(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    auto sqval = generic_cast<std::remove_reference_t<TV>, SQObjectPtr>(vm, val);
                    set(sqkey, sqval);
                }

                template <>
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->clazz;

                    sq_pushobject(vm, self);
                    sq_pushobject(vm, sqkey);
                    sq_pushobject(vm, sqval);
                    sq_newslot(vm, -3, SQTrue);
                    sq_pop(vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = generic_cast<SQObjectPtr, std::remove_reference_t<TV>>(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->clazz;
                    if (!vm->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }

        };
    }
}
