#pragma once

#include "definition.h"
#include "pydict.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/types/sqclass.hpp"
#include "sqbinding/pybinding/common/cast.h"

namespace sqbinding {
namespace python {
class Class : public detail::Class, public std::enable_shared_from_this<Class> {
  public:
    // bindFunc to current class
    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        // TODO: 实装支持 withenv
        set(funcname, std::forward<Func>(func));
    }

  public:
    // link to a existed table in vm stack
    Class(::SQClass *pClass, detail::VM vm) : detail::Class(pClass, vm) {
    }
    void bind_this_if_need(PyValue &v);
    PyValue get(PyValue key) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        auto v = detail::Class::get<PyValue, PyValue>(key);
        bind_this_if_need(v);
        return v;
    }

    // Python Interface
    SQInteger __len__() {
        return 0;
        // return pClass->CountUsed();
    }
    PyValue __getitem__(PyValue key) {
        return std::move(get(key));
    }
    PyValue __setitem__(PyValue key, PyValue val) {
        set(key, val);
        return val;
    }
    py::list keys() {
        detail::VM &vm = holder->vm;
        return std::move(sqbinding::python::Table(pClass()->_members, vm).keys());
    }

    std::string __str__() {
        return string_format("OT_CLASS: [addr={%p}, ref=%d]", pClass(), getRefCount());
    }

    std::string __repr__() {
        return "SQClass(" + __str__() + ")";
    }
};
} // namespace python
} // namespace sqbinding
