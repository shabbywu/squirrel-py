#include "definition.h"
#include "container.h"


PyValue _SQClass_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (_class(obj)->Get(sqkey, sqval)) {
        return sqobject_topython(sqval, vm);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue _SQClass_::getAttributes(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (_class(obj)->GetAttributes(sqkey, sqval)) {
        return sqobject_topython(sqval, vm);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue _SQClass_::set(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    _class(obj)->NewSlot(_ss(vm), sqkey, sqval, 0);
    return val;
}

PyValue _SQClass_::setAttributes(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (_class(obj)->SetAttributes(sqkey, sqval)) {
        return val;
    } else {
        _class(obj)->NewSlot(_ss(vm), sqkey, sqval, 0);
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
    return _SQTable_(_class(obj)->_members, vm).keys();
}

void _SQClass_::bindFunc(std::string funcname, py::function func) {
    set(funcname, func);
}
