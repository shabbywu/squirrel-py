#ifndef _SQBINDING_CLOSURE_H_
#define _SQBINDING_CLOSURE_H_

#include <squirrel.h>
#include "definition.h"

class _SQClosure_  {
public:
    SQObjectPtr handler;
    HSQUIRRELVM vm = nullptr;
    SQClosure* pClosure;
    SQObjectPtr pthis; // 'this' pointer for sq_call

    // link to a existed table in vm stack
    _SQClosure_ (SQClosure* pClosure, HSQUIRRELVM vm): pClosure(pClosure), vm(vm), handler(pClosure) {
        sq_addref(vm, &handler);
    }

    _SQClosure_(const _SQClosure_& rhs) {
        pClosure = rhs.pClosure;
        vm = rhs.vm;
        handler = pClosure;
        sq_addref(vm, &handler);
    }
    _SQClosure_& operator=(const _SQClosure_& rhs) {
        release();
        pClosure = rhs.pClosure;
        vm = rhs.vm;
        handler = pClosure;
        sq_addref(vm, &handler);
        return *this;
    };

    ~_SQClosure_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << pClosure -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
    }

    // Python Interface
    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue get(PyValue key);
    PyValue __call__(py::args args);
    std::string __str__() {
        return string_format("OT_CLOSURE: [addr={%p}, ref=%d]", pClosure, pClosure->_uiRef);
    }
    std::string __repr__() {
        return "SQClosure(" + __str__() + ")";
    }
};

class _SQNativeClosure_  {
public:
    SQObjectPtr handler;
    HSQUIRRELVM vm = nullptr;
    SQNativeClosure* pNativeClosure;
    SQObjectPtr pthis; // 'this' pointer for sq_call

    _SQNativeClosure_(std::shared_ptr<py::function> func, HSQUIRRELVM vm, SQFUNCTION caller): vm(vm) {
        pNativeClosure = SQNativeClosure::Create(_ss(vm), caller, 1);
        pNativeClosure->_nparamscheck = 0;
        SQUserPointer ptr = sq_newuserdata(vm, sizeof(py::function));
        std::memcpy(ptr, func.get(), sizeof(py::function));
        pNativeClosure->_outervalues[0] = vm->PopGet();
        handler = pNativeClosure;
        sq_addref(vm, &handler);
    }

    // link to a existed table in vm stack
    _SQNativeClosure_ (SQNativeClosure* pNativeClosure, HSQUIRRELVM vm): pNativeClosure(pNativeClosure), vm(vm), handler(pNativeClosure)  {
        sq_addref(vm, &handler);
    }

    _SQNativeClosure_(const _SQNativeClosure_& rhs) {
        pNativeClosure = rhs.pNativeClosure;
        vm = rhs.vm;
        handler = pNativeClosure;
        sq_addref(vm, &handler);
    }
    _SQNativeClosure_& operator=(const _SQNativeClosure_& rhs) {
        release();
        pNativeClosure = rhs.pNativeClosure;
        vm = rhs.vm;
        handler = pNativeClosure;
        sq_addref(vm, &handler);
        return *this;
    };

    ~_SQNativeClosure_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << this -> pNativeClosure -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
    }

    // Python Interface
    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue get(PyValue key);
    PyValue __call__(py::args args);
    std::string __str__() {
        return string_format("OT_NATIVECLOSURE: [addr={%p}, ref=%d]", pNativeClosure, pNativeClosure->_uiRef);
    }
    std::string __repr__() {
        return "SQNativeClosure(" + __str__() + ")";
    }
};
#endif
