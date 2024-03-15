#ifndef _SQBINDING_FUNCTION_H_
#define _SQBINDING_FUNCTION_H_

#include <squirrel.h>
#include "object.h"

// PythonNativeCall: wrapper for python function, this function will be used to call python func in SQVM and return result to SQVM
SQInteger PythonNativeCall(HSQUIRRELVM vm);

class _SQClosure_: public _SQObjectPtr_  {
public:

    SQObjectPtr pthis; // 'this' pointer for sq_call

    // link to a existed table in vm stack
    _SQClosure_ (SQObjectPtr& closure, HSQUIRRELVM vm, bool releaseOnDestroy = true) :  _SQObjectPtr_(closure, vm, releaseOnDestroy) {
    }

    _SQClosure_ (SQClosure* closure, HSQUIRRELVM vm) : _SQObjectPtr_(vm, false) {
        obj = SQObjectPtr(closure);
    }

    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue __call__(py::args args);
};


class _SQNativeClosure_: public _SQObjectPtr_  {
public:

    SQObjectPtr pthis; // 'this' pointer for sq_call

    // link to a existed table in vm stack
    _SQNativeClosure_ (SQObjectPtr& closure, HSQUIRRELVM vm, bool releaseOnDestroy = true) :  _SQObjectPtr_(closure, vm, releaseOnDestroy) {
    }

    _SQNativeClosure_ (SQNativeClosure* closure, HSQUIRRELVM vm) : _SQObjectPtr_(vm, false) {
        obj = SQObjectPtr(closure);
    }

    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue __call__(py::args args);
};


#endif
