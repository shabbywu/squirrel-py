#include "definition.h"
#include "container.h"


PyValue sqbinding::python::Class::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->clazz;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
            c->bindThis(self);
        }
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
            c->bindThis(self);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue sqbinding::python::Class::getAttributes(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (pClass()->GetAttributes(sqkey, sqval)) {
        return sqobject_topython(sqval, vm);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue sqbinding::python::Class::set(PyValue key, PyValue val) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    std::cout << "call class set key: " << sqobject_to_string(sqkey) << " , value: " << sqobject_to_string(sqval) << std::endl;
    pClass()->NewSlot(_ss(vm), sqkey, sqval, 1);
    return val;
}

PyValue sqbinding::python::Class::setAttributes(PyValue key, PyValue val) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (pClass()->SetAttributes(sqkey, sqval)) {
        return val;
    } else {
        pClass()->NewSlot(_ss(vm), sqkey, sqval, 0);
        return val;
    }
}

PyValue sqbinding::python::Class::__getitem__(PyValue key) {
    return std::move(get(key));
}

PyValue sqbinding::python::Class::__setitem__(PyValue key, PyValue val) {
    return std::move(set(key, val));
}

py::list sqbinding::python::Class::keys() {
    HSQUIRRELVM& vm = holder->vm;
    return std::move(sqbinding::python::Table(pClass()->_members, vm).keys());
}

void sqbinding::python::Class::bindFunc(std::string funcname, py::function func) {
    set(funcname, func);
}
