#include "definition.h"
#include "container.h"
#include "sqbinding/pybinding/common/cast.h"

namespace py = pybind11;

PyValue sqbinding::python::Closure::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->closure;
    auto v = detail::Closure<PyValue (py::args)>::get<PyValue, PyValue>(key);
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
    return v;
}



PyValue sqbinding::python::NativeClosure::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->nativeClosure;
    auto v = detail::NativeClosure<PyValue (py::args)>::get<PyValue, PyValue>(key);
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
    return v;
}
