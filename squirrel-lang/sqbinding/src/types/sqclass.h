#ifndef _SQBINDING_CLASS_H_
#define _SQBINDING_CLASS_H_

#include "definition.h"
#include "sqiterator.h"
#include "pydict.h"


class _SQClass_ : public std::enable_shared_from_this<_SQClass_>  {
public:
    SQObjectPtr handler;
    SQClass* pClass;
    HSQUIRRELVM vm = nullptr;
    bool releaseOnDestroy = false;

    // link to a existed table in vm stack
    _SQClass_ (SQClass* pClass, HSQUIRRELVM vm) : pClass(pClass), vm(vm), handler(pClass) {
        sq_addref(vm, &handler);
    }

    _SQClass_(const _SQClass_& rhs) {
        pClass = rhs.pClass;
        vm = rhs.vm;
        handler = pClass;
        sq_addref(vm, &handler);
    }
    _SQClass_& operator=(const _SQClass_& rhs) {
        release();
        pClass = rhs.pClass;
        vm = rhs.vm;
        handler = pClass;
        sq_addref(vm, &handler);
    };

    ~_SQClass_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << pClass -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
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
        return string_format("OT_CLASS: [addr={%p}, ref=%d]", pClass, pClass->_uiRef);
    }

    std::string __repr__() {
        return "SQClass(" + __str__() + ")";
    }
};
#endif
