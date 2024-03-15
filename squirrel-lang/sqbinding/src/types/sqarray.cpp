#include "definition.h"
#include "must.h"


PyValue _SQArray_::__getitem__(int idx) {
    SQObjectPtr sqval;
    if (pArray->Get(idx, sqval)) {
        return sqobject_topython(sqval, vm);
    }
    throw py::index_error(string_format("%d", idx).c_str());
}

PyValue _SQArray_::__setitem__(int idx, PyValue val) {
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (pArray->Set(idx, sqval)) {
        return val;
    }
    throw std::runtime_error(string_format("can't set idx=%d", idx) + " to value=" + sqobject_to_string(sqval));
}

PyValue _SQArray_::append(PyValue val) {
    pArray->Append(pyvalue_tosqobject(val, vm));
    return val;
}


PyValue _SQArray_::pop() {
    if (pArray->Size() < 1) {
        throw std::runtime_error("can't pop empty array");
    }
    SQObjectPtr sqval = pArray->Top();
    pArray->Pop();
    return sqobject_topython(sqval, vm);
}