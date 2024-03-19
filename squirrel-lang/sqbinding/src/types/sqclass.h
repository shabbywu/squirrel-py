#ifndef _SQBINDING_CLASS_H_
#define _SQBINDING_CLASS_H_

#include "definition.h"
#include "sqiterator.h"
#include "pydict.h"


class _SQClass_: public _SQObjectPtr_ {
public:

    // link to a existed table in vm stack
    _SQClass_ (SQObjectPtr& pclass, HSQUIRRELVM vm, bool releaseOnDestroy = true) : _SQObjectPtr_(pclass, vm, releaseOnDestroy) {
    }

    _SQClass_ (SQClass* pclass, HSQUIRRELVM vm) : _SQObjectPtr_(vm, false) {
        obj = SQObjectPtr(pclass);
    }

    PyValue get(PyValue key);
    PyValue set(PyValue key, PyValue val);
    PyValue getAttributes(PyValue key);
    PyValue setAttributes(PyValue key, PyValue val);
    // bindFunc to current class
    void bindFunc(std::string funcname, py::function func);

    // Python Interface
    SQInteger __len__() {
        return _table(obj)->CountUsed();
    }
    PyValue __getitem__(PyValue key);
    PyValue __setitem__(PyValue key, PyValue val);
    py::list keys();
};
#endif
