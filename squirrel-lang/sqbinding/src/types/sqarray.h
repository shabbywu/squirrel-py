#ifndef _SQBINDING_ARRAY_H_
#define _SQBINDING_ARRAY_H_

#include "definition.h"
#include "sqiterator.h"
#include "object.h"


class _SQArray_ : public std::enable_shared_from_this<_SQArray_> {
public:
    SQObjectPtr handler;
    SQArray* pArray;
    HSQUIRRELVM vm;
    bool releaseOnDestroy = false;

    // create a array in vm stack
    _SQArray_ (HSQUIRRELVM vm): vm(vm) {
        pArray = SQArray::Create(_ss(vm), 4);
        handler = pArray;
        sq_addref(vm, &handler);
    }

    // link to a existed table in vm stack
    _SQArray_ (SQArray* pArray, HSQUIRRELVM vm): pArray(pArray), vm(vm), handler(pArray) {
        sq_addref(vm, &handler);
    }

    _SQArray_(const _SQArray_& rhs) {
        pArray = rhs.pArray;
        vm = rhs.vm;
        handler = pArray;
        sq_addref(vm, &handler);
    }
    _SQArray_& operator=(const _SQArray_& rhs) {
        release();
        pArray = rhs.pArray;
        vm = rhs.vm;
        handler = pArray;
        sq_addref(vm, &handler);
    };

    ~_SQArray_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << pArray -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
    }

    // Python Interface
    PyValue __getitem__(int idx);
    PyValue __setitem__(int idx, PyValue val);
    PyValue append(PyValue val);
    PyValue pop();
    ArrayIterator __iter__() {
        return ArrayIterator(this);
    }
    SQInteger __len__() {
        return pArray->Size();
    }


    std::string __str__() {
        return string_format("OT_ARRAY: [addr={%p}, ref=%d]", pArray, pArray->_uiRef);
    }

    std::string __repr__() {
        return "SQArray(" + __str__() + ")";
    }
};
#endif
