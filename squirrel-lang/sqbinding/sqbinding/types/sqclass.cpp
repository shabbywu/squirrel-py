#include "definition.h"
#include "container.h"


PyValue _SQClass_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = {pClass};
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

PyValue _SQClass_::getAttributes(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (pClass->GetAttributes(sqkey, sqval)) {
        return sqobject_topython(sqval, vm);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue _SQClass_::set(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    std::cout << "call class set key: " << sqobject_to_string(sqkey) << " , value: " << sqobject_to_string(sqval) << std::endl;
    pClass->NewSlot(_ss(vm), sqkey, sqval, 1);
    return val;
}

PyValue _SQClass_::setAttributes(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (pClass->SetAttributes(sqkey, sqval)) {
        return val;
    } else {
        pClass->NewSlot(_ss(vm), sqkey, sqval, 0);
        return val;
    }
}

PyValue _SQClass_::__getitem__(PyValue key) {
    return std::move(get(key));
}

PyValue _SQClass_::__setitem__(PyValue key, PyValue val) {
    return std::move(set(key, val));
}

py::list _SQClass_::keys() {
    return std::move(sqbinding::python::Table(pClass->_members, vm).keys());
}

void _SQClass_::bindFunc(std::string funcname, py::function func) {
    set(funcname, func);
}
