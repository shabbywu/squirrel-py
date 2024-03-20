#ifndef _SQBINDING_ARRAY_H_
#define _SQBINDING_ARRAY_H_

#include "definition.h"
#include "sqiterator.h"
#include "object.h"


class _SQArray_ : public std::enable_shared_from_this<_SQArray_> {
public:
    SQArray* pArray;
    HSQUIRRELVM vm;
    bool releaseOnDestroy = false;

    // create a array in vm stack
    _SQArray_ (HSQUIRRELVM vm) {
        SQObjectPtr obj;
        sq_newarray(vm, 0);
        sq_getstackobj(vm,-1,&obj);
        sq_addref(vm, &obj);
        sq_pop(vm,1);
        pArray = _array(obj);
        releaseOnDestroy = true;
        this->vm = vm;
    }

    // link to a existed table in vm stack
    _SQArray_ (SQArray* pArray, HSQUIRRELVM vm) {
        this->pArray = pArray;
        this -> pArray -> _uiRef++;
        this->vm = vm;
    }

    _SQArray_(const _SQArray_& rhs) {
        this -> pArray = rhs.pArray;
        this -> pArray -> _uiRef++;
        this -> vm = rhs.vm;
    }
    _SQArray_& operator=(const _SQArray_& rhs) {
        release();
        this -> pArray = rhs.pArray;
        this -> pArray -> _uiRef++;
        this -> vm = rhs.vm;
    };

    ~_SQArray_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release _SQArray_" << std::endl;
        #endif
        if(releaseOnDestroy) {
            pArray->Release();
        } else {
            this -> pArray -> _uiRef--;
        }
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
};
#endif
