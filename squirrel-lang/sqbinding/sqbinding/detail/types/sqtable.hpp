#pragma once

#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/cast_impl.hpp"
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"
#include "sqbinding/detail/types/sqfunction.hpp"
#include "sqvm.hpp"
#include "holder.hpp"


namespace sqbinding {
    namespace detail {
        class Table: public std::enable_shared_from_this<Table> {
            using Holder = SQObjectPtrHolder<::SQTable*>;
            public:
                std::shared_ptr<Holder> holder;
            public:
                Table(VM vm): holder(std::make_shared<Holder>(SQTable::Create(_ss(*vm), 4), vm)) {}
                Table(::SQTable* pTable, VM vm): holder(std::make_shared<Holder>(pTable, vm)) {}

                Table(const Table &o)
                {
                    std::cout << "Copying sqbinding::detial::Table" << std::endl;
                    holder = o.holder;
                }

                SQUnsignedInteger getRefCount() {
                    return pTable() -> _uiRef;
                }

                ::SQTable* pTable() {
                    return _table(holder->GetSQObjectPtr());
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
            public:
                // bindFunc to current table
                template<class Func>
                void bindFunc(std::string funcname, Func&& func, bool withenv = false) {
                    set(funcname,
                    detail::NativeClosure<detail::function_signature_t<Func>>::template Create<detail::cpp_function, Func>(
                        func, holder->GetVM(), detail::cpp_function::caller
                    ));
                }
        };
    }
}
