#ifndef _SQBINDING_FUNCTION_H_
#define _SQBINDING_FUNCTION_H_

#include <squirrel.h>
#include "definition.h"

// PythonNativeCall: wrapper for python function, this function will be used to call python func in SQVM and return result to SQVM
SQInteger PythonNativeCall(HSQUIRRELVM vm);

class _SQClosure_  {
public:
    HSQUIRRELVM vm;
    SQObjectPtr obj;
    SQClosure* pClosure;
    SQObjectPtr pthis; // 'this' pointer for sq_call

    // link to a existed table in vm stack
    _SQClosure_ (SQClosure* pClosure, HSQUIRRELVM vm) {
        this -> pClosure = pClosure;
        this -> pClosure -> _uiRef++;
        this -> vm = vm;
    }

    _SQClosure_(const _SQClosure_& rhs) {
        this -> pClosure = rhs.pClosure;
        this -> pClosure -> _uiRef++;
        this -> vm = rhs.vm;
    }
    _SQClosure_& operator=(const _SQClosure_& rhs) {
        release();
        this -> pClosure = rhs.pClosure;
        this -> pClosure -> _uiRef++;
        this -> vm = rhs.vm;
    };


    ~_SQClosure_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release _SQClosure_ uiRef--" << std::endl;
        #endif
        this -> pClosure -> _uiRef--;
    }

    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue __call__(py::args args);
};

class _SQNativeClosure_  {
public:

    HSQUIRRELVM vm;
    SQObjectPtr obj;
    SQNativeClosure* pNativeClosure;
    SQObjectPtr pthis; // 'this' pointer for sq_call

    // link to a existed table in vm stack
    _SQNativeClosure_ (SQNativeClosure* pNativeClosure, HSQUIRRELVM vm) {
        this -> pNativeClosure = pNativeClosure;
        this -> pNativeClosure -> _uiRef++;
        this -> vm = vm;
    }

    _SQNativeClosure_(const _SQNativeClosure_& rhs) {
        this -> pNativeClosure = rhs.pNativeClosure;
        this -> pNativeClosure -> _uiRef++;
        this -> vm = rhs.vm;
    }
    _SQNativeClosure_& operator=(const _SQNativeClosure_& rhs) {
        release();
        this -> pNativeClosure = rhs.pNativeClosure;
        this -> pNativeClosure -> _uiRef++;
        this -> vm = rhs.vm;
    };


    ~_SQNativeClosure_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release _SQNativeClosure_ uiRef--" << std::endl;
        #endif
        this -> pNativeClosure -> _uiRef--;
    }

    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue __call__(py::args args);
};


#endif
