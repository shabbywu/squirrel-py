#ifndef _VM_H_
#define _VM_H_

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>
#include <iostream>
#include <cstring>
#include <memory.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "types/definition.h"


namespace py = pybind11;
void printStdout(HSQUIRRELVM vm, const SQChar *format,...);
void printStdErr(HSQUIRRELVM vm, const SQChar *format,...);
void printCompileError(HSQUIRRELVM, const SQChar * desc, const SQChar * source, SQInteger line, SQInteger column);

class StaticVM {
public:
    HSQUIRRELVM vm;
    std::shared_ptr<_SQTable_> roottable;

    StaticVM() {};
    StaticVM(HSQUIRRELVM vm) {
        this->vm = vm;
        vmlock::register_vm_handle(vm);
    }

    StaticVM(const StaticVM& rhs) {
        this -> vm = rhs.vm;
        vmlock::register_vm_handle(vm);
    }
    StaticVM& operator=(const StaticVM& rhs) {
        release();
        this -> vm = rhs.vm;
        vmlock::register_vm_handle(vm);
    };

    virtual void release() {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release StaticVM: " << vm << std::endl;
        #endif
        vmlock::unregister_vm_handle(vm);
    }

    ~StaticVM() {
        release();
    }

    void DumpStack(SQInteger stackbase, bool dumpall);

    SQInteger gettop();
    void settop(SQInteger top);
    std::shared_ptr<_SQTable_> getroottable();
    void setroottable(std::shared_ptr<_SQTable_> roottable);
    void pushrootable();

    PyValue ExecuteString(std::string sourcecode, PyValue env = py::none());
    PyValue ExecuteBytecode(std::string bytecode, PyValue env = py::none());

    _SQObjectPtr_* StackTop();
    void bindFunc(std::string funcname, py::function func);

    inline std::string GetLastError() {
        const SQChar* sqErr;
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL) {
            sq_pop(vm, 1);
            return std::string();
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 2);
        return std::string(sqErr);
    };

};


void register_squirrel_vm(py::module_ &m);
void set_static_vm(HSQUIRRELVM vm);
#endif
