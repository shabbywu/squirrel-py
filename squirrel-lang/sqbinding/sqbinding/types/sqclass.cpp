#include "definition.h"
#include "container.h"
#include "sqbinding/common/cast.h"


PyValue sqbinding::python::Class::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->clazz;
    auto v = detail::Class::get<PyValue, PyValue>(key);
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

PyValue sqbinding::python::Class::__getitem__(PyValue key) {
    return std::move(get(key));
}

PyValue sqbinding::python::Class::__setitem__(PyValue key, PyValue val) {
    set(key, val);
    return val;
}

py::list sqbinding::python::Class::keys() {
    HSQUIRRELVM& vm = holder->vm;
    return std::move(sqbinding::python::Table(pClass()->_members, vm).keys());
}

void sqbinding::python::Class::bindFunc(std::string funcname, PyValue func) {
    set(PyValue(funcname), PyValue(func));
}
