#pragma once
#include <memory>
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqvm.hpp"
#include "holder.hpp"

namespace sqbinding {
    namespace detail {
        class Class: public std::enable_shared_from_this<Class> {
            using Holder = SQObjectPtrHolder<::SQClass*>;
            public:
                std::shared_ptr<Holder> holder;
            public:
                Class (::SQClass* pClass, VM vm): holder(std::make_shared<Holder>(pClass, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pClass() -> _uiRef;
                }

                ::SQClass* pClass() {
                    return _class(holder->GetSQObjectPtr());
                }

            public:
                template <typename TK, typename TV>
                void set(TK& key, TV& val) {
                    VM& vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::cast(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    VM& vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::cast(vm, val);
                    set(sqkey, sqval);
                }

                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_newslot(*vm, -3, SQTrue);
                    sq_pop(*vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    VM& vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
            // bindFunc to current class
            template <typename Func>
            void bindFunc(std::string funcname, Func func) {
                set<std::string, Func>(funcname, func);
            }
        };
    }
}
