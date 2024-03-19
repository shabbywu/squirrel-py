#ifndef _SQBINDING_INSTANCE_H_
#define _SQBINDING_INSTANCE_H_

#include "definition.h"
#include "sqiterator.h"
#include "pydict.h"


class _SQInstance_ : public std::enable_shared_from_this<_SQInstance_>  {
public:
    SQInstance* pInstance;
    HSQUIRRELVM vm;

    // link to a existed pInstance in vm stack
    _SQInstance_ (SQInstance* pInstance, HSQUIRRELVM vm) {
        this->pInstance = pInstance;
        this->pInstance -> _uiRef++;
        this->vm = vm;
    }

    _SQInstance_(const _SQInstance_& rhs) {
        this -> pInstance = rhs.pInstance;
        this -> pInstance -> _uiRef++;
        this -> vm = rhs.vm;
    }
    _SQInstance_& operator=(const _SQInstance_& rhs) {
        release();
        this -> pInstance = rhs.pInstance;
        this -> pInstance -> _uiRef++;
        this -> vm = rhs.vm;
    };

    ~_SQInstance_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release _SQInstance_" << std::endl;
        #endif
        this -> pInstance -> _uiRef--;
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
        return string_format("OT_INSTANCE: [{%p}]", pInstance);
    }

    std::string __repr__() {
        return "SQInstance(" + __str__() + ")";
    }
};
#endif
