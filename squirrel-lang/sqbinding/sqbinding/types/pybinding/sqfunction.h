#pragma once

#include <squirrel.h>
#include "sqbinding/common/format.h"
#include "sqbinding/common/call_setup.hpp"
#include "definition.h"
#include "sqbinding/types/cppbinding/sqfunction.hpp"

namespace sqbinding {
    namespace python {
        class Closure: public detail::Closure<PyValue (py::args)> {
            public:
                Closure(::SQClosure* pClosure, HSQUIRRELVM vm): detail::Closure<PyValue (py::args)>(pClosure, vm) {
                };
            public:
                // Python API
                PyValue get(PyValue key);
                PyValue __call__(py::args args) {
                    return this->operator()(args);
                }
                std::string __str__() {
                    return to_string();
                }
                std::string __repr__() {
                    return "Closure(" + to_string() + ")";
                }
        };
    }
}

namespace sqbinding {
    namespace python {
        class NativeClosure: public detail::NativeClosure<PyValue (py::args)> {
            public:
                NativeClosure(::SQNativeClosure* pNativeClosure, HSQUIRRELVM vm): detail::NativeClosure<PyValue (py::args)>(pNativeClosure, vm) {
                };
                NativeClosure(std::shared_ptr<py::function> func, HSQUIRRELVM vm, SQFUNCTION caller): detail::NativeClosure<PyValue (py::args)>(::SQNativeClosure::Create(_ss(vm), caller, 1), vm) {
                    pNativeClosure()->_nparamscheck = 0;
                    SQUserPointer ptr = sq_newuserdata(vm, sizeof(py::function));
                    std::memcpy(ptr, func.get(), sizeof(py::function));
                    pNativeClosure()->_outervalues[0] = vm->PopGet();
                }

            public:
                // Python API
                PyValue get(PyValue key);
                PyValue __call__(py::args args) {
                    return this->operator()(args);
                }
                std::string __str__() {
                    return to_string();
                }
                std::string __repr__() {
                    return "NativeClosure(" + to_string() + ")";
                }
        };
    }
}
