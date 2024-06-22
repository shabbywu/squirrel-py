#pragma once
#include <sqobject.h>
#include "definition.h"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/detail/types/sqobject.hpp"

namespace sqbinding {
    namespace python {
        class ObjectPtr: public detail::ObjectPtr {
            public:
                ObjectPtr(::SQObjectPtr& pObject, detail::VM vm): detail::ObjectPtr(pObject, vm) {};
                ObjectPtr(::SQObjectPtr&& pObject, detail::VM vm): detail::ObjectPtr(pObject, vm) {};

            PyValue to_python() {
                detail::VM& vm = holder->vm;
                return detail::GenericCast<PyValue(SQObjectPtr&)>::template cast(vm, **this);
            }
            void from_python(PyValue val) {
                detail::VM& vm = holder->vm;
                holder = std::make_shared<detail::ObjectPtr::Holder>(detail::GenericCast<SQObjectPtr(PyValue)>::template cast(vm, val), vm);
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
