#ifndef _SQBINDING_INSTANCE_H_
#define _SQBINDING_INSTANCE_H_

#include "definition.h"
#include "sqiterator.h"
#include "pydict.h"


class _SQInstance_ : public std::enable_shared_from_this<_SQInstance_>  {
public:
    SQObjectPtr handler;
    SQInstance* pInstance;
    HSQUIRRELVM vm = nullptr;

    // link to a existed pInstance in vm stack
    _SQInstance_ (SQInstance* pInstance, HSQUIRRELVM vm): pInstance(pInstance), vm(vm), handler(pInstance) {
        sq_addref(vm, &handler);
    }

    _SQInstance_(const _SQInstance_& rhs) {
        pInstance = rhs.pInstance;
        vm = rhs.vm;
        handler = pInstance;
        sq_addref(vm, &handler);
    }
    _SQInstance_& operator=(const _SQInstance_& rhs) {
        release();
        pInstance = rhs.pInstance;
        vm = rhs.vm;
        handler = pInstance;
        sq_addref(vm, &handler);
    };

    ~_SQInstance_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << this -> pInstance -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
    }

    PyValue get(PyValue key);
    PyValue set(PyValue key, PyValue val);
    PyValue getAttributes(PyValue key);
    PyValue setAttributes(PyValue key, PyValue val);
    // bindFunc to current instance
    void bindFunc(std::string funcname, py::function func);

    // Python Interface
    PyValue __getitem__(PyValue key);
    PyValue __setitem__(PyValue key, PyValue val);
    py::list keys();

    std::string __str__() {
        return string_format("OT_INSTANCE: [addr={%p}, ref=%d]", pInstance, pInstance->_uiRef);
    }

    std::string __repr__() {
        return "SQInstance(" + __str__() + ")";
    }
};
#endif
