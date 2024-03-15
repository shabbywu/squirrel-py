#include "definition.h"
#include "object.h"


PyValue _SQObjectPtr_::to_python() {
    return sqobject_topython(obj, vm);
}


void _SQObjectPtr_::from_python(PyValue val) {
    obj = pyvalue_tosqobject(val, vm);
    return;
}


std::string _SQObjectPtr_::__str__() {
    return sqobject_to_string(obj);
}

std::string _SQObjectPtr_::__repr__() {
    return "SQObjectPtr(" + sqobject_to_string(obj) + ")";
}