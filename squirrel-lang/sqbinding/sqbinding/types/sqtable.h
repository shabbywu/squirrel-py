#ifndef _SQBINDING_TABLE_H_
#define _SQBINDING_TABLE_H_

#include "definition.h"
#include "sqiterator.h"
#include "object.h"


class _SQTable_ : public std::enable_shared_from_this<_SQTable_> {
public:
    SQObjectPtr handler;
    SQTable* pTable;
    HSQUIRRELVM vm = nullptr;

    // create a table in vm stack
    _SQTable_ (HSQUIRRELVM vm): vm(vm) {
        pTable = SQTable::Create(_ss(vm), 4);
        handler = pTable;
        sq_addref(vm, &handler);
    }

    // link to a existed table in vm stack
    _SQTable_ (SQTable* pTable, HSQUIRRELVM vm) : pTable(pTable), vm(vm), handler(pTable) {
        sq_addref(vm, &handler);
    }

    _SQTable_(const _SQTable_& rhs) {
        pTable = rhs.pTable;
        vm = rhs.vm;
        handler = pTable;
        sq_addref(vm, &handler);
    }

    _SQTable_& operator=(const _SQTable_& rhs) {
        release();
        pTable = rhs.pTable;
        vm = rhs.vm;
        handler = pTable;
        sq_addref(vm, &handler);
        return *this;
    };

    ~_SQTable_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << pTable -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
    }

    PyValue get(PyValue key);
    void set(SQObjectPtr& key, SQObjectPtr& val);
    PyValue set(PyValue key, PyValue val);
    // bindFunc to current table
    void bindFunc(std::string funcname, py::function func);
    void bindFunc(std::string funcname, std::shared_ptr<sqbinding::python::NativeClosure> func);

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
