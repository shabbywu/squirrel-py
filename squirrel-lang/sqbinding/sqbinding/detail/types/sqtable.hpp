#pragma once

#include "holder.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqbinding/detail/sqdefinition.hpp"
#include "sqbinding/detail/types/sqfunction.hpp"
#include "sqvm.hpp"

namespace sqbinding {
namespace detail {
class TableIterator
    : public std::iterator<std::input_iterator_tag, std::tuple<SQObjectPtr, SQObjectPtr>, SQInteger,
                           std::tuple<SQObjectPtr, SQObjectPtr> *, std::tuple<SQObjectPtr, SQObjectPtr>> {
    using Holder = SQObjectPtrHolder<::SQTable *>;

  public:
    std::shared_ptr<Holder> holder;
    SQInteger idx = 0;

  public:
    TableIterator(::SQTable *pTable, detail::VM vm) : holder(std::make_shared<Holder>(pTable, vm)), idx(0) {
        ++*this;
    }
    TableIterator(std::shared_ptr<Holder> holder, SQInteger idx) : holder(holder), idx(idx - 1) {
        ++*this;
    }

  public:
    ::SQTable *pTable() const {
        return _table(holder->GetSQObjectPtr());
    }
    SQInteger capacity() const {
        return pTable()->_numofnodes;
    }

  public:
    TableIterator &operator++() {
        idx++;
        while (idx < capacity()) {
            auto &n = pTable()->_nodes[idx];
            if (sq_type(n.key) != tagSQObjectType::OT_NULL) {
                break;
            }
            idx++;
        }
        return *this;
    }
    TableIterator operator++(int) {
        TableIterator retval = *this;
        ++(*this);
        return retval;
    }
    bool operator==(TableIterator other) const {
        return holder == other.holder && idx == other.idx;
    }
    bool operator!=(TableIterator other) const {
        return !(*this == other);
    }
    reference operator*() const {
        if (idx >= capacity()) {
            throw sqbinding::stop_iteration();
        }
        VM &vm = holder->GetVM();
        auto &n = pTable()->_nodes[idx];
        SQObjectPtr &key = n.key;
        SQObjectPtr &value = n.val;
        return std::make_tuple<SQObjectPtr &, SQObjectPtr &>(key, value);
    }

    TableIterator begin() {
        return TableIterator(holder, 0);
    }
    TableIterator end() {
        return TableIterator(holder, capacity());
    }
};

class Table : public std::enable_shared_from_this<Table> {
    using Holder = SQObjectPtrHolder<::SQTable *>;
    using ErrNotFound = sqbinding::key_error;
  public:
    std::shared_ptr<Holder> holder;

  public:
    Table(VM vm) : holder(std::make_shared<Holder>(SQTable::Create(_ss(*vm), 4), vm)) {
    }
    Table(::SQTable *pTable, VM vm) : holder(std::make_shared<Holder>(pTable, vm)) {
    }

    SQUnsignedInteger getRefCount() {
        return pTable()->_uiRef;
    }

    ::SQTable *pTable() {
        return _table(holder->GetSQObjectPtr());
    }
    // SQInteger size() {
    //     return pTable()->_usednodes;
    // }
    SQInteger capacity() {
        return pTable()->_numofnodes;
    }
    TableIterator iterator() {
        return TableIterator(holder, 0);
    }

  public:
    SQOBJECTPTR_SETTER_TEMPLATE
    void set(SQObjectPtr &sqkey, SQObjectPtr &sqval) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();

        sq_pushobject(*vm, self);
        sq_pushobject(*vm, sqkey);
        sq_pushobject(*vm, sqval);
        sq_newslot(*vm, -3, SQFalse);
        sq_pop(*vm, 1);
    }

  public:
    SQOBJECTPTR_GETTER_TEMPLATE
  protected:
    bool get(SQObjectPtr &key, SQObjectPtr &ret) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
            return false;
        }
        return true;
    }

  protected:
    std::map<std::string, std::shared_ptr<detail::generic_function>> functions;

  public:
    // bindFunc to current table
    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        auto p = functions.find(funcname);
        if (p == functions.end()) {
            auto wrapper = to_cpp_function(std::forward<Func>(func));
            functions[funcname] = wrapper;
            set(std::forward<std::string>(funcname),
                std::move(detail::NativeClosure<detail::function_signature_t<Func>>::Create(wrapper, holder->GetVM())));
        } else {
            if (auto overloaded = dynamic_cast<detail::overloaded_function *>(p->second.get())) {
                overloaded->add_caller(std::forward<Func>(func));
            } else {
                auto wrapper = std::make_shared<detail::overloaded_function>();
                wrapper->add_caller(p->second);
                wrapper->add_caller(std::forward<Func>(func));
                functions[funcname] = wrapper;
                set(std::forward<std::string>(funcname),
                    std::move(
                        detail::NativeClosure<detail::function_signature_t<Func>>::Create(wrapper, holder->GetVM())));
            }
        }
    }
};
} // namespace detail
} // namespace sqbinding
