#include "definition.h"
#include "container.h"
#include "sqtable.h"


PyValue _SQInstance_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = {pInstance};
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
            auto p = SQObjectPtr(pInstance);
            c->bindThis(p);
        }
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
            auto p = SQObjectPtr(pInstance);
            c->bindThis(p);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue _SQInstance_::set(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    SQObjectPtr self = {pInstance};
    if (vm->Set(self, sqkey, sqval, DONT_FALL_BACK)) {
        return val;
    } else if (vm->NewSlot(self, sqkey, sqval, false)) {
        return val;
    }
    throw std::runtime_error("can't set key=" + sqobject_to_string(sqkey) + " to value=" + sqobject_to_string(sqval));
}

PyValue _SQInstance_::__getitem__(PyValue key) {
    return get(key);
}

PyValue _SQInstance_::__setitem__(PyValue key, PyValue val) {
    return set(key, val);
}

py::list _SQInstance_::keys() {
    SQInteger idx = 0;
    py::list keys;
    auto table = pInstance->_class->_members;
    while (idx < table->_numofnodes) {
        auto n = &table->_nodes[idx++];
        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
            keys.append(sqobject_topython(n->key, vm));
        }
    }
    return keys;
}

void _SQInstance_::bindFunc(std::string funcname, py::function func) {
    set(funcname, func);
}
