#pragma once
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqvm.hpp"


namespace sqbinding {
    namespace detail {
        class Instance: public std::enable_shared_from_this<Instance> {
            public:
                struct Holder {
                    Holder(::SQInstance* pInstance, VM vm) : vm(vm) {
                        instance = pInstance;
                        sq_addref(*vm, &instance);
                    }
                    ~Holder(){
                        sq_release(*vm, &instance);
                    }
                    VM vm;
                    SQObjectPtr instance;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                Instance (::SQInstance* pInstance, VM vm): holder(std::make_shared<Holder>(pInstance, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pInstance() -> _uiRef;
                }
                ::SQInstance* pInstance() {
                    return _instance(holder->instance);
                }
            public:
                template <typename TK, typename TV>
                void set(TK& key, TV& val) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::cast(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::cast(vm, val);
                    set(sqkey, sqval);
                }

                template <>
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->instance;

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_newslot(*vm, -3, SQFalse);
                    sq_pop(*vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->instance;
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
            // bindFunc to current instance
            template <typename Func>
            void bindFunc(std::string funcname, Func func) {
                set<std::string, Func>(funcname, func);
            }
        };
    }
}
