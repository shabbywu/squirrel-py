#include "definition.h"
#include "container.h"
#include "sqtable.h"
#include "sqbinding/common/cast.h"


PyValue sqbinding::python::Instance::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->instance;
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
    throw py::key_error(detail::sqobject_to_string(sqkey));
}

// PyValue sqbinding::python::Instance::set(PyValue key, PyValue val) {
//     HSQUIRRELVM& vm = holder->vm;
//     SQObjectPtr& self = holder->instance;
//     SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
//     SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
//     if (vm->Set(self, sqkey, sqval, DONT_FALL_BACK)) {
//         return val;
//     } else if (vm->NewSlot(self, sqkey, sqval, false)) {
//         return val;
//     }
//     throw std::runtime_error("can't set key=" + detail::sqobject_to_string(sqkey) + " to value=" + detail::sqobject_to_string(sqval));
// }

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
    set(PyValue(funcname), func);
}
