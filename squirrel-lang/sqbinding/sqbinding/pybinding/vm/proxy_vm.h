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
#include "sqbinding/detail/vm/vm.hpp"

namespace py = pybind11;
namespace sqbinding {
    namespace python {
        class VMProxy: public detail::VMProxy {
        public:
            std::shared_ptr<python::Table> roottable;
        public:
            VMProxy() = delete;
            VMProxy(HSQUIRRELVM vm): detail::VMProxy(vm) {}
        public:
            std::shared_ptr<python::Table>& getroottable() {
                if (roottable == nullptr) {
                    roottable = std::make_shared<python::Table>(_table(GetSQVM()->_roottable), GetVM());
                }
                return roottable;
            }
        };
    }
}
