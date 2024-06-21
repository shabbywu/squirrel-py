#include "definition.h"
#include "container.h"
#include "sqbinding/common/cast.h"


PyValue sqbinding::python::Array::__getitem__(int idx) {
    return get<int, PyValue>(idx);
}

PyValue sqbinding::python::Array::__setitem__(int idx, PyValue val) {
    set<int, PyValue>(idx, val);
    return val;
}

PyValue sqbinding::python::ArrayIterator::__next__() {
    if (idx < 0) {
        throw py::stop_iteration();
    }
    PyValue result;
    try {
        result = obj->__getitem__(idx);
    } catch(const py::index_error& e) {
        throw py::stop_iteration();
    }
    idx++;
    return result;
}
