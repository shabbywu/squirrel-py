#pragma once

#include "definition.h"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/detail/types/sqobject.hpp"

namespace sqbinding {
    namespace python {
        class ObjectPtr: public detail::ObjectPtr {
            public:
                ObjectPtr(::SQObjectPtr& pObject, HSQUIRRELVM vm): detail::ObjectPtr(pObject, vm) {};
                ObjectPtr(::SQObjectPtr&& pObject, HSQUIRRELVM vm): detail::ObjectPtr(pObject, vm) {};

            PyValue to_python() {
                HSQUIRRELVM& vm = holder->vm;
                return detail::generic_cast<SQObjectPtr, PyValue>(vm, **this);
            }
            void from_python(PyValue val) {
                HSQUIRRELVM& vm = holder->vm;
                holder = std::make_shared<detail::ObjectPtr::Holder>(detail::generic_cast<PyValue, SQObjectPtr>(vm, val), vm);
                return;
            }

            std::string __str__() {
                return detail::sqobject_to_string(**this);
            }

            std::string __repr__() {
                return "SQObjectPtr(" + detail::sqobject_to_string(**this) + ")";
            }
        };
    }
}
