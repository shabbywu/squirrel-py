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
        class TableIterator: public std::iterator<
                                        std::input_iterator_tag,
                                        std::tuple<SQObjectPtr, SQObjectPtr>,
                                        SQInteger,
                                        std::tuple<SQObjectPtr, SQObjectPtr>*,
                                        std::tuple<SQObjectPtr, SQObjectPtr>
                                    > {
            using Holder = SQObjectPtrHolder<::SQTable*>;

            public:
                std::shared_ptr<Holder> holder;
                SQInteger idx = 0;
            public:
                TableIterator(::SQTable* pTable, detail::VM vm): holder(std::make_shared<Holder>(pTable, vm)), idx(0) {
                    ++*this;
                }
                TableIterator(std::shared_ptr<Holder> holder, SQInteger idx): holder(holder), idx(idx-1) {
                    ++*this;
                }
            public:
                ::SQTable* pTable() const {
                    return _table(holder->GetSQObjectPtr());
                }
                SQInteger capacity() const {
                    return pTable()->_numofnodes;
                }
            public:
                TableIterator& operator++() {
                    idx ++;
                    while (idx < capacity()) {
                        auto& n = pTable()->_nodes[idx];
                        if (sq_type(n.key) != tagSQObjectType::OT_NULL) {
                            break;
                        }
                        idx ++;
                    }
                    return *this;
                }
                TableIterator operator++(int) {
                    TableIterator retval = *this; ++(*this); return retval;
                }
                bool operator==(TableIterator other) const {
                    return holder == other.holder && idx == other.idx;
                }
                bool operator!=(TableIterator other) const { return !(*this == other); }
                reference operator*() const {
                    if (idx >= capacity()) {
                        throw sqbinding::stop_iteration();
                    }
                    VM& vm = holder->GetVM();
                    auto&n = pTable()->_nodes[idx];
                    SQObjectPtr& key = n.key;
                    SQObjectPtr& value = n.val;
                    return std::make_tuple<SQObjectPtr&, SQObjectPtr&>(key, value);
                }

                TableIterator begin() { return TableIterator(holder, 0); }
                TableIterator end() {
                    return TableIterator(holder, capacity());
                }
        };

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
                // SQInteger size() {
                //     return pTable()->_usednodes;
                // }
                SQInteger capacity() {
                    return pTable()->_numofnodes;
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
                    detail::NativeClosure<detail::function_signature_t<Func>>::template Create<detail::cpp_function<2>, Func>(
                        std::forward<Func>(func), holder->GetVM(), detail::cpp_function<2>::caller
                    ));
                }

                /// bindFunc a cpp_function from a class method (non-const, no ref-qualifier)
                template <typename Return, typename Class, typename... Args>
                void bindFunc(std::string funcname, Return (Class::*func)(Args...), bool withenv = false) {
                    using Func = Return(Class*, Args...);
                    set(funcname,
                    detail::NativeClosure<Func>::template Create<detail::cpp_function<1>>(
                        func, holder->GetVM(), detail::cpp_function<1>::caller
                    ));
                }
        };
    }
}
