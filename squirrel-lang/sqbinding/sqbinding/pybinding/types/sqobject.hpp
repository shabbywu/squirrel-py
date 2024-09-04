#pragma once
#include "definition.h"
#include "sqbinding/detail/types/sqobject.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include <sqobject.h>

namespace sqbinding {
namespace python {
class ObjectPtr : public detail::ObjectPtr {
  public:
    ObjectPtr(::SQObjectPtr &pObject, detail::VM vm) : detail::ObjectPtr(pObject, vm) {};
    ObjectPtr(::SQObjectPtr &&pObject, detail::VM vm) : detail::ObjectPtr(pObject, vm) {};

    PyValue to_python() {
        detail::VM &vm = holder->vm;
        return detail::generic_cast<SQObjectPtr, PyValue>(vm, std::forward<SQObjectPtr>(**this));
    }
    void from_python(PyValue val) {
        detail::VM &vm = holder->vm;
        holder = std::make_shared<detail::ObjectPtr::Holder>(
            detail::generic_cast<PyValue, SQObjectPtr>(vm, std::forward<PyValue>(val)), vm);
        return;
    }

    std::string __str__() {
        return detail::sqobject_to_string(**this);
    }

    std::string __repr__() {
        return "SQObjectPtr(" + detail::sqobject_to_string(**this) + ")";
    }
};
} // namespace python
} // namespace sqbinding
