#pragma once
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/types/definition.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <squirrel.h>

namespace py = pybind11;

namespace sqbinding {
namespace detail {
template <> inline void generic_stack_push<py::args &&>(VM vm, py::args &&args) {
    // push args into stack
    for (auto var_ : args) {
        SQObjectPtr var = generic_cast<PyValue, SQObjectPtr>(vm, std::forward<PyValue>(var_.cast<PyValue>()));
        sq_pushobject(*vm, std::move(var));
    }
}

template <> inline void generic_stack_push<py::args &>(VM vm, py::args &args) {
    // push args into stack
    for (auto var_ : args) {
        SQObjectPtr var = generic_cast<PyValue, SQObjectPtr>(vm, std::forward<PyValue>(var_.cast<PyValue>()));
        sq_pushobject(*vm, std::move(var));
    }
}
} // namespace detail
} // namespace sqbinding
