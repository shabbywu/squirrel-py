#ifndef _SQBINDING_POINTER_H_
#define _SQBINDING_POINTER_H_

#include "definition.h"

class _SQObjectPtr_ {
public:
    HSQUIRRELVM vm;
    SQObjectPtr obj;
    bool releaseOnDestroy = false;

    _SQObjectPtr_(HSQUIRRELVM vm, bool releaseOnDestroy) {
        sq_resetobject(&obj);
        this->vm = vm;
        this->releaseOnDestroy = releaseOnDestroy;
    }

    _SQObjectPtr_(SQObjectPtr& ptr, HSQUIRRELVM vm): _SQObjectPtr_(ptr, vm, true) {}

    _SQObjectPtr_(SQObjectPtr& ptr, HSQUIRRELVM vm, bool releaseOnDestroy) {
        sq_resetobject(&obj);
        this->obj = ptr;
        this->vm = vm;
        this->releaseOnDestroy = releaseOnDestroy;
        if (releaseOnDestroy) sq_addref(vm, &obj);
    }

    SQObjectType type() {
        return this->obj._type;
    }

    SQObjectPtr& operator* () {
        return (this->obj);
    }

    ~_SQObjectPtr_() {
        if (releaseOnDestroy) {
            Release();
            releaseOnDestroy = false;
        }
    }

    virtual void Release() {
        sq_release(vm, &obj);
        sq_resetobject(&obj);
    }

    PyValue to_python();
    void from_python(PyValue val);

    std::string __str__();
    std::string __repr__();
};

#endif
