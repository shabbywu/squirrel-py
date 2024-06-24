#pragma once
#include "sqbinding/detail/common/call_setup.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "cast.h"


namespace py = pybind11;


namespace sqbinding {
    namespace detail {
        template <int index>
        struct load_args<index, py::list>{
            static py::list load(VM vm) {
                int nparams = sq_gettop(*vm) - 2;
                py::list args;
                for (int idx = index; idx <= 1 + nparams; idx ++) {
                    args.append(generic_stack_get<PyValue>(vm, idx));
                }
                return args;
            }
        };
    }
}
