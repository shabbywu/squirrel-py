#pragma once
#include "cast.h"
#include "sqbinding/detail/common/call_setup.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace sqbinding {
namespace detail {
// python dynamic_args_function args loader
//
// Example to call py::function with args in sqvm stack
// template<class Return, class... Args>
// Return call_func(py::function func) {
//    py::list args = load_args<Args...>(vm);
//    return func(*args);
// }
template <int index> struct load_args<index, py::list> {
    static py::list load(VM vm) {
        int nparams = sq_gettop(*vm) - 2;
        py::list args;
        for (int idx = index; idx <= 1 + nparams; idx++) {
            args.append(generic_stack_get<PyValue>(vm, idx));
        }
        return args;
    }
};
} // namespace detail
} // namespace sqbinding
