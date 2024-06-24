#pragma once
#include <memory>
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
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
                SQOBJECTPTR_SETTER_TEMPLATE
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
                SQOBJECTPTR_GETTER_TEMPLATE
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
            void bindFunc(std::string funcname, Func func, bool withenv = false) {
                set<std::string, Func>(funcname, func);
            }
        };
    }
}
