#include "definition.h"
#include "container.h"


PyValue sqbinding::python::Array::__getitem__(int idx) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->array;
    SQObjectPtr sqkey = (SQInteger)idx;
    SQObjectPtr sqval;
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        return sqobject_topython(sqval, vm);
    }
    throw py::index_error(string_format("%d", idx).c_str());
}

PyValue sqbinding::python::Array::__setitem__(int idx, PyValue val) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->array;
    SQObjectPtr sqkey = (SQInteger)idx;
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (vm->Set(self, sqkey, sqval, DONT_FALL_BACK)) {
        return val;
    }
    throw std::runtime_error(string_format("can't set idx=%d", idx) + " to value=" + sqobject_to_string(sqval));
}

PyValue sqbinding::python::Array::append(PyValue val) {
    HSQUIRRELVM& vm = holder->vm;
    pArray()->Append(pyvalue_tosqobject(val, vm));
    return val;
}


PyValue sqbinding::python::Array::pop() {
    HSQUIRRELVM& vm = holder->vm;
    if (pArray()->Size() < 1) {
        throw std::runtime_error("can't pop empty array");
    }
    SQObjectPtr sqval = pArray()->Top();
    pArray()->Pop();
    return sqobject_topython(sqval, vm);
}
