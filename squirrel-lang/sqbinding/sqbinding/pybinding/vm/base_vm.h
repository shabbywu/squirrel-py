#pragma once
#include <iostream>
#include <cstring>
#include <memory.h>

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>

#include <chrono>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sqbinding/pybinding/types/definition.h"
#include "sqbinding/pybinding/types/container.h"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/detail/vm/printer.hpp"
#include "sqbinding/detail/common/debug.hpp"
#include "sqbinding/detail/vm/base_vm.hpp"

namespace py = pybind11;
namespace sqbinding {
    namespace python {
        class BaseVM: public detail::BaseVM {
        public:
            std::shared_ptr<python::Table> roottable;
        public:
            BaseVM(): detail::BaseVM() {}
            BaseVM(HSQUIRRELVM vm): detail::BaseVM(vm) {}

            std::shared_ptr<python::Table>& getroottable() {
                if (roottable == nullptr) {
                    roottable = std::make_shared<python::Table>(_table(GetSQVM()->_roottable), GetVM());
                }
                return roottable;
            }

            void bindFunc(std::string funcname, py::function func) {
                getroottable()->bindFunc(funcname, func);
            }
        };
    }
}
