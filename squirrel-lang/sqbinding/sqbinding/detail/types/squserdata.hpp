#pragma once

#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/cast_impl.hpp"
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqvm.hpp"
#include "holder.hpp"
#include "sqtable.h"


namespace sqbinding {
    namespace detail {
        template <class T>
        class UserData: public std::enable_shared_from_this<UserData> {
            using Holder = SQObjectPtrHolder<::SQUserData*>;
            public:
                std::shared_ptr<Holder> holder;
                std::shared_ptr<sqbinding::detail::Table> delegate;
            public:
                UserData(::SQUserData* pUserData, VM vm): holder(std::make_shared<Holder>(pUserData, vm)) {}
                UserData(::SQUserData* pUserData, sqbinding::detail::Table delegate, VM vm): holder(std::make_shared<Holder>(pUserData, vm), delegate(delegate)) {}

                SQUnsignedInteger getRefCount() {
                    return pUserData() -> _uiRef;
                }

                ::SQUserData* pUserData() {
                    return _userdata(holder->GetSQObjectPtr());
                }

                std::shared_ptr<sqbinding::detail::Table> GetDelegate() {
                    return delegate;
                }
                void SetDelegate(std::shared_ptr<sqbinding::detail::Table> delegate) {
                    this->delegate = delegate;
                }
            public:
                SQOBJECTPTR_SETTER_TEMPLATE
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_newslot(*vm, -3, SQFalse);
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
        };
    }
}
