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
            using detail::VMProxy::VMProxy;
            ~VMProxy(){
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release python::VMProxy: " << GetSQVM() << std::endl;
                #endif
                roottable = nullptr;
            }
        public:
            std::shared_ptr<python::Table>& getroottable() {
                if (roottable == nullptr) {
                    roottable = std::make_shared<python::Table>(_table(GetSQVM()->_roottable), GetVM());
                }
                return roottable;
            }
            // FIXME: 让 bindfunc 支持绑定 python 方法?
            template <class Func>
            void bindFunc(std::string funcname, Func&& func, bool withenv = false) {
                getroottable()->bindFunc(funcname, func, withenv);
            }
        };
    }
}
