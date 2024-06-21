#pragma once

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
#include "sqbinding/types/definition.h"
#include "sqbinding/types/container.h"
#include "sqbinding/common/errors.h"
#include "sqbinding/common/cast.h"
#include "printer.h"

namespace py = pybind11;
namespace sqbinding {
    namespace detail {
        class BaseVM {
            public:
                struct Holder {
                    Holder(HSQUIRRELVM vm) : vm(vm) {
                        vmlock::register_vm_handle(vm);
                        roottable = std::make_shared<sqbinding::python::Table>(_table(vm->_roottable), vm);
                    }
                    ~Holder(){
                        #ifdef TRACE_CONTAINER_GC
                        std::cout << "GC::Release BaseVM: " << vm << std::endl;
                        #endif
                        vmlock::unregister_vm_handle(vm);
                    }
                    HSQUIRRELVM vm;
                    std::shared_ptr<sqbinding::python::Table> roottable;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                BaseVM() = default;
                BaseVM(HSQUIRRELVM vm) : holder(std::make_shared<Holder>(vm)) {}
            public:
                void DumpStack(SQInteger stackbase, bool dumpall);

                SQInteger gettop();
                void settop(SQInteger top);
                std::shared_ptr<sqbinding::python::Table> getroottable();
                std::shared_ptr<sqbinding::python::ObjectPtr> StackTop();
                HSQUIRRELVM& GetVM() {return holder->vm;}

                inline std::string GetLastError() {
                    HSQUIRRELVM& vm = holder->vm;
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
    }

    namespace python {
        class BaseVM: public detail::BaseVM {
        public:
            BaseVM(): detail::BaseVM() {}
            BaseVM(HSQUIRRELVM vm): detail::BaseVM(vm) {}

            PyValue ExecuteString(std::string sourcecode, PyValue env = py::none());
            PyValue ExecuteBytecode(std::string bytecode, PyValue env = py::none());

            void bindFunc(std::string funcname, py::function func);

        };
    }
}
