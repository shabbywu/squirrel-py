#include "definition.h"
#include "sqcontainer.h"


PyValue _SQTable_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (pTable->Get(sqkey, sqval)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<_SQClosure_>>(v)) {
            auto c = std::get<std::shared_ptr<_SQClosure_>>(v);
            auto p = SQObjectPtr(pTable);
            c->bindThis(p);
        }
        if (std::holds_alternative<std::shared_ptr<_SQNativeClosure_>>(v)) {
            auto c = std::get<std::shared_ptr<_SQNativeClosure_>>(v);
            auto p = SQObjectPtr(pTable);
            c->bindThis(p);
        }
        return v;
    }
    throw py::key_error(sqobject_to_string(sqkey));
}


PyValue _SQTable_::set(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (pTable->Set(sqkey, sqval)) {
        return val;
    } else {
        pTable->NewSlot(sqkey, sqval);
        return val;
    }
}


PyValue _SQTable_::__getitem__(PyValue key) {
    return get(key);
}

PyValue _SQTable_::__setitem__(PyValue key, PyValue val) {
    return set(key, val);
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
