#pragma once

#include "sqbinding/detail/common/format.hpp"
#include "definition.h"
#include "pydict.hpp"
#include "sqbinding/detail/types/sqclass.hpp"

namespace sqbinding {
    namespace python {
        class Class: public detail::Class, public std::enable_shared_from_this<Class> {
            public:
            // link to a existed table in vm stack
            Class (::SQClass* pClass, HSQUIRRELVM vm): detail::Class(pClass, vm) {}

            PyValue get(PyValue key);
            // bindFunc to current class
            void bindFunc(std::string funcname, PyValue func) {
                set(funcname, func);
            }

            // Python Interface
            SQInteger __len__() {
                return 0;
                // return pClass->CountUsed();
            }
            PyValue __getitem__(PyValue key) {
                return std::move(get(key));
            }
            PyValue __setitem__(PyValue key, PyValue val) {
                set(key, val);
                return val;
            }
            py::list keys() {
                HSQUIRRELVM& vm = holder->vm;
                return std::move(sqbinding::python::Table(pClass()->_members, vm).keys());
            }


            std::string __str__() {
                return string_format("OT_CLASS: [addr={%p}, ref=%d]", pClass(), getRefCount());
            }

            std::string __repr__() {
                return "SQClass(" + __str__() + ")";
            }
        };
    }
}
