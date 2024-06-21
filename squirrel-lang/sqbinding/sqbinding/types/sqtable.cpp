#include "definition.h"
#include "container.h"
#include "sqbinding/common/cast.h"


PyValue sqbinding::python::Table::get(PyValue& key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->table;
    auto v = detail::Table::get<PyValue, PyValue>(key);
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
    return v;
};


PyValue sqbinding::python::Table::__getitem__(PyValue key) {
    return get(key);
}

PyValue sqbinding::python::Table::__setitem__(PyValue key, PyValue val) {
    set(key, val);
    return val;
}

void sqbinding::python::Table::__delitem__(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    pTable()->Remove(sqkey);
}


py::list sqbinding::python::Table::keys() {
    HSQUIRRELVM& vm = holder->vm;
    SQInteger idx = 0;
    py::list keys;
    while (idx < pTable()->_numofnodes) {
        auto n = &pTable()->_nodes[idx++];
        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
            keys.append(sqobject_topython(n->key, vm));
        }
    }
    return keys;
}


void sqbinding::python::Table::bindFunc(std::string funcname, PyValue func) {
    set(PyValue(funcname), func);
}


PyValue sqbinding::python::TableIterator::__next__() {
    if (idx < 0) {
        throw py::stop_iteration();
    }
    PyValue key;
    PyValue value;
    bool found;
    HSQUIRRELVM& vm = obj->holder->vm;
    while (idx < obj->pTable()->_numofnodes) {
        auto n = &obj->pTable()->_nodes[idx++];
        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
            key = sqobject_topython(n->key, vm);
            value = sqobject_topython(n->val, vm);
            found = 1;
            break;
        }
    }
    if (!found) {
        throw py::stop_iteration();
    }
    return py::make_tuple(key, value);
}
