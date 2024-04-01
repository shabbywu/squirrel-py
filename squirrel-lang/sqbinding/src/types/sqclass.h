#ifndef _SQBINDING_CLASS_H_
#define _SQBINDING_CLASS_H_

#include "definition.h"
#include "sqiterator.h"
#include "pydict.h"


class _SQClass_ : public std::enable_shared_from_this<_SQClass_>  {
public:
    SQClass* pClass;
    HSQUIRRELVM vm = nullptr;
    bool releaseOnDestroy = false;

    // link to a existed table in vm stack

    _SQClass_ (SQClass* pClass, HSQUIRRELVM vm) : pClass(pClass), vm(vm) {
        pClass->_uiRef ++;
    }

    _SQClass_(const _SQClass_& rhs) {
        this -> pClass = rhs.pClass;
        this -> pClass -> _uiRef++;
        this -> vm = rhs.vm;
    }
    _SQClass_& operator=(const _SQClass_& rhs) {
        release();
        this -> pClass = rhs.pClass;
        this -> pClass -> _uiRef++;
        this -> vm = rhs.vm;
    };

    ~_SQClass_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release _SQClass_ uiRef--=" << this -> pClass -> _uiRef - 1 << std::endl;
        #endif
        this -> pClass -> _uiRef--;
        if(releaseOnDestroy && this-> pClass -> _uiRef == 0) {
            #ifdef TRACE_CONTAINER_GC
            std::cout << "GC::Release _SQClass_ release" << std::endl;
            #endif
            pClass->Release();
        }
    }


    PyValue get(PyValue key);
    PyValue set(PyValue key, PyValue val);
    PyValue getAttributes(PyValue key);
    PyValue setAttributes(PyValue key, PyValue val);
    // bindFunc to current class
    void bindFunc(std::string funcname, py::function func);

    // Python Interface
    SQInteger __len__() {
        return 0;
        // return pClass->CountUsed();
    }
    PyValue __getitem__(PyValue key);
    PyValue __setitem__(PyValue key, PyValue val);
    py::list keys();


    std::string __str__() {
        return string_format("OT_CLASS: [{%p}]", pClass);
    }

    std::string __repr__() {
        return "SQClass(" + __str__() + ")";
    }
};
#endif
