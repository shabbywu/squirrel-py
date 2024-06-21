#pragma once
#include <squirrel.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sqbinding/common/cast.h"
#include "sqbinding/types/pybinding/definition.h"

namespace py = pybind11;


namespace sqbinding {
    namespace detail {
        class stack_guard {
            public:
                stack_guard(HSQUIRRELVM v) {
                    vm = v;
                    top = sq_gettop(vm);
                }
                ~stack_guard() {
                    sq_settop(vm, top);
                }
            private:
                HSQUIRRELVM vm;
                SQInteger top;

            public:
                int offset() {
                    return sq_gettop(vm) - top;
                }
        };

        template <class Arg> inline
        void generic_stack_push(HSQUIRRELVM vm, Arg arg) {
            sq_pushobject(vm, arg);
        }

        template <> inline
        void generic_stack_push<py::args>(HSQUIRRELVM vm, py::args args) {
            // push args into stack
            for (auto var_ : args) {
                auto var = python::pyvalue_tosqobject(std::move(var_.cast<PyValue>()), vm);
                sq_pushobject(vm, std::move(var));
            }
        }

        template <class Return> inline
        Return generic_stack_get(HSQUIRRELVM vm, SQInteger index) {
            HSQOBJECT ref;
            sq_getstackobj(vm, index, &ref);
            return generic_cast<HSQOBJECT, Return>(vm, ref);
        }

    }
}
