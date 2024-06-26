#pragma once
#include <squirrel.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqbinding/pybinding/types/definition.h"

namespace py = pybind11;


namespace sqbinding {
    namespace detail {
        template <> inline
        void generic_stack_push<py::args>(VM vm, py::args args) {
            // push args into stack
            for (auto var_ : args) {
                auto var = python::pyvalue_tosqobject(std::move(var_.cast<PyValue>()), vm);
                sq_pushobject(*vm, std::move(var));
            }
        }

    }
}
