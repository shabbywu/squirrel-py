#include "definition.h"
#include "container.h"


PyValue _SQTable_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = {pTable};
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<_SQClosure_>>(v)) {
            auto& c = std::get<std::shared_ptr<_SQClosure_>>(v);
            c->bindThis(self);
        }
        if (std::holds_alternative<std::shared_ptr<_SQNativeClosure_>>(v)) {
            auto& c = std::get<std::shared_ptr<_SQNativeClosure_>>(v);
            c->bindThis(self);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}


void _SQTable_::set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
    SQObjectPtr self = {pTable};
    if (vm->Set(self, sqkey, sqval, DONT_FALL_BACK)) {
        return;
    } else if (vm->NewSlot(self, sqkey, sqval, false)) {
        return;
    }
    throw std::runtime_error("can't set key=" + sqobject_to_string(sqkey) + " to value=" + sqobject_to_string(sqval));
}


PyValue _SQTable_::set(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    set(sqkey, sqval);
    return val;
}


PyValue _SQTable_::__getitem__(PyValue key) {
    auto v = std::move(get(key));
    return std::move(v);
}

PyValue _SQTable_::__setitem__(PyValue key, PyValue val) {
    return std::move(set(key, val));
}

void _SQTable_::__delitem__(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    pTable->Remove(sqkey);
}


py::list _SQTable_::keys() {
    SQInteger idx = 0;
    py::list keys;
    while (idx < pTable->_numofnodes) {
        auto n = &pTable->_nodes[idx++];
        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
            keys.append(sqobject_topython(n->key, vm));
        }
    }
    return keys;
}

void _SQTable_::bindFunc(std::string funcname, py::function func) {
    set(funcname, func);
}

void _SQTable_::bindFunc(std::string funcname, std::shared_ptr<_SQNativeClosure_> func) {
    set(funcname, func);
}
