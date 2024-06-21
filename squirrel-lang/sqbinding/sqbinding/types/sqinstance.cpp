#include "definition.h"
#include "container.h"
#include "sqtable.h"
#include "sqbinding/common/cast.h"


PyValue sqbinding::python::Instance::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->instance;
    auto v = detail::Instance::get<PyValue, PyValue>(key);
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

PyValue sqbinding::python::Instance::__getitem__(PyValue& key) {
    return get(key);
}

PyValue sqbinding::python::Instance::__setitem__(PyValue key, PyValue val) {
    set<PyValue, PyValue>(key, val);
    return val;
}

py::list sqbinding::python::Instance::keys() {
    HSQUIRRELVM& vm = holder->vm;
    SQInteger idx = 0;
    py::list keys;
    auto table = pInstance()->_class->_members;
    while (idx < table->_numofnodes) {
        auto n = &table->_nodes[idx++];
        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
            keys.append(sqobject_topython(n->key, vm));
        }
    }
    return keys;
}

void sqbinding::python::Instance::bindFunc(std::string funcname, PyValue func) {
    set(PyValue(funcname), PyValue(func));
}
