#include "definition.h"
#include "container.h"


PyValue sqbinding::python::Table::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->table;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
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


void sqbinding::python::Table::set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->table;
    if (vm->Set(self, sqkey, sqval, DONT_FALL_BACK)) {
        return;
    } else if (vm->NewSlot(self, sqkey, sqval, false)) {
        return;
    }
    throw std::runtime_error("can't set key=" + sqobject_to_string(sqkey) + " to value=" + sqobject_to_string(sqval));
}


PyValue sqbinding::python::Table::set(PyValue key, PyValue val) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    set(sqkey, sqval);
    return val;
}


PyValue sqbinding::python::Table::__getitem__(PyValue key) {
    auto v = std::move(get(key));
    return std::move(v);
}

PyValue sqbinding::python::Table::__setitem__(PyValue key, PyValue val) {
    return std::move(set(key, val));
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
    set(funcname, func);
}
