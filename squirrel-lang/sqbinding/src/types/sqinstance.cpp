#include "definition.h"
#include "container.h"


PyValue _SQInstance_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    if (pInstance->Get(sqkey, sqval)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<_SQClosure_>>(v)) {
            auto c = std::get<std::shared_ptr<_SQClosure_>>(v);
            auto p = SQObjectPtr(pInstance);
            c->bindThis(p);
        }
        if (std::holds_alternative<std::shared_ptr<_SQNativeClosure_>>(v)) {
            auto c = std::get<std::shared_ptr<_SQNativeClosure_>>(v);
            auto p = SQObjectPtr(pInstance);
            c->bindThis(p);
        }
        return v;
    }
    throw py::key_error(sqobject_to_string(sqkey));
}

PyValue _SQInstance_::set(PyValue key, PyValue val) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval = pyvalue_tosqobject(val, vm);
    if (pInstance->Set(sqkey, sqval)) {
        return val;
    } else {
        SQObjectPtr res;
        SQObjectPtr closure;
        if (pInstance -> _delegate && pInstance -> _delegate->Get((*_ss(vm)->_metamethods)[MT_NEWSLOT],closure)) {
            SQObjectPtr p(pInstance);
            vm->Push(p);
            vm->Push(sqkey);
            vm->Push(sqval);
            if(vm->CallMetaMethod(closure, MT_NEWSLOT, 3, res)) {
                return val;
            }
        }
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
    std::cout << "instance: " << (void*) pInstance << std::endl;
    std::cout << "instance._class: " << (void*) pInstance->_class << std::endl;
    std::cout << "instance._class._members: " << (void*) pInstance->_class->_members << std::endl;
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
