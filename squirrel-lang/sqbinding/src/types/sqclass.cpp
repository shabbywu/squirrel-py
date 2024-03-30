#include "definition.h"
#include "container.h"


PyValue _SQClass_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (pClass->Get(sqkey, sqval)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<_SQClosure_>>(v)) {
            auto c = std::get<std::shared_ptr<_SQClosure_>>(v);
            auto p = SQObjectPtr(pClass);
            c->bindThis(p);
        }
        if (std::holds_alternative<std::shared_ptr<_SQNativeClosure_>>(v)) {
            auto c = std::get<std::shared_ptr<_SQNativeClosure_>>(v);
            auto p = SQObjectPtr(pClass);
            c->bindThis(p);
        }
        return v;
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
    pClass->NewSlot(_ss(vm), sqkey, sqval, 0);
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
    return get(key);
}

PyValue _SQClass_::__setitem__(PyValue key, PyValue val) {
    return set(key, val);
}

py::list _SQClass_::keys() {
    return _SQTable_(pClass->_members, vm).keys();
}

void _SQClass_::bindFunc(std::string funcname, py::function func) {
    set(funcname, func);
}
