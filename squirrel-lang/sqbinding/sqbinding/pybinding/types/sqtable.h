#pragma once

#include "definition.h"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/types/sqtable.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/common/stack_operation.h"

namespace sqbinding {
namespace python {
class TableIterator : public detail::TableIterator {
  public:
    using detail::TableIterator::TableIterator;
    PyValue __next__() {
        auto ret = *((*this)++);
        auto vm = holder->GetVM();
        return py::make_tuple(
            detail::generic_cast<SQObjectPtr, PyValue>(vm, std::forward<SQObjectPtr>(std::get<0>(ret))),
            detail::generic_cast<SQObjectPtr, PyValue>(vm, std::forward<SQObjectPtr>(std::get<1>(ret)))
        );
    }
};
class Table : public detail::Table, public std::enable_shared_from_this<Table> {
  public:
    Table(detail::VM vm) : detail::Table(vm) {
    }
    Table(::SQTable *pTable, detail::VM vm) : detail::Table(pTable, vm) {
    }

  public:
    // bindFunc to current table
    // FIXME: 让 bindfunc 只支持绑定 python 方法?
    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        // TODO: 实装支持 withenv
        set<std::string, Func>(std::forward<std::string>(funcname), std::forward<Func>(func));
    }
    void bind_this_if_need(PyValue &v);

  public:
    // Python API
    PyValue get(PyValue &key) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        PyValue v = detail::Table::get<PyValue, PyValue>(std::forward<PyValue>(key));
        bind_this_if_need(v);
        return v;
    }
    py::list keys() {
        detail::VM &vm = holder->vm;
        py::list keys;
        for (auto [k, _] : __iter__()) {
            keys.append(detail::generic_cast<SQObjectPtr, PyValue>(vm, std::forward<SQObjectPtr>(k)));
        }
        return keys;
    }

    TableIterator __iter__() {
        return TableIterator(holder, 0);
    }
    SQInteger __len__() {
        return pTable()->CountUsed();
    }
    PyValue __getitem__(PyValue key) {
        return get(key);
    }
    PyValue __setitem__(PyValue key, PyValue val) {
        set(std::forward<PyValue>(key), std::forward<PyValue>(val));
        return val;
    }
    void __delitem__(PyValue key) {
        // TODO: add pop(key)
        detail::VM &vm = holder->vm;
        SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
        pTable()->Remove(sqkey);
    }

    std::string __str__() {
        return string_format("OT_TABLE: [addr={%p}, ref=%d]", pTable(), pTable()->_uiRef);
    }

    std::string __repr__() {
        return "SQTable(" + __str__() + ")";
    }
};
} // namespace python
} // namespace sqbinding
