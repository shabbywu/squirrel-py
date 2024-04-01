#ifndef _SQBINDING_TABLE_H_
#define _SQBINDING_TABLE_H_

#include "definition.h"
#include "sqiterator.h"
#include "object.h"


class _SQTable_ : public std::enable_shared_from_this<_SQTable_> {
public:
    SQTable* pTable;
    HSQUIRRELVM vm = nullptr;
    bool releaseOnDestroy = false;

    // create a table in vm stack
    _SQTable_ (HSQUIRRELVM vm) {
        this -> pTable = SQTable::Create(_ss(vm), 4);
        this -> pTable -> _uiRef++;
        this -> vm = vm;
        this -> releaseOnDestroy = true;
    }

    // link to a existed table in vm stack
    _SQTable_ (SQTable* pTable, HSQUIRRELVM vm) {
        this -> pTable = pTable;
        this -> pTable -> _uiRef++;
        this -> vm = vm;
    }

    _SQTable_(const _SQTable_& rhs) {
        this -> pTable = rhs.pTable;
        this -> pTable -> _uiRef++;
        this -> vm = rhs.vm;
    }
    _SQTable_& operator=(const _SQTable_& rhs) {
        release();
        this -> pTable = rhs.pTable;
        this -> pTable -> _uiRef++;
        this -> vm = rhs.vm;
    };

    ~_SQTable_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << this -> pTable -> _uiRef -1 << std::endl;
        #endif
        this -> pTable -> _uiRef--;
        if(releaseOnDestroy && this-> pTable -> _uiRef == 0) {
            #ifdef TRACE_CONTAINER_GC
            std::cout << "GC::Release _SQTable_ release" << std::endl;
            #endif
            pTable->Release();
        }
    }

    PyValue get(PyValue key);
    PyValue set(PyValue key, PyValue val);
    // bindFunc to current table
    void bindFunc(std::string funcname, py::function func);
    SQUnsignedInteger getRefCount() {
        return this -> pTable -> _uiRef;
    }

    // Python Interface
    TableIterator __iter__() {
        return TableIterator(this);
    }
    SQInteger __len__() {
        return pTable->CountUsed();
    }
    PyValue __getitem__(PyValue key);
    PyValue __setitem__(PyValue key, PyValue val);
    void __delitem__(PyValue key);
    py::list keys();

    std::string __str__() {
        return string_format("OT_TABLE: [addr={%p}, ref=%d]", pTable, pTable->_uiRef);
    }

    std::string __repr__() {
        return "SQTable(" + __str__() + ")";
    }
};

#endif
