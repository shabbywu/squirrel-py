#include "definition.h"
#include "sqobject.h"


PyValue sqbinding::python::ObjectPtr::to_python() {
    HSQUIRRELVM& vm = holder->vm;
    return sqobject_topython(**this, vm);
}


void sqbinding::python::ObjectPtr::from_python(PyValue val) {
    HSQUIRRELVM& vm = holder->vm;
    holder = std::make_shared<detail::ObjectPtr::Holder>(pyvalue_tosqobject(val, vm), vm);
    return;
}
