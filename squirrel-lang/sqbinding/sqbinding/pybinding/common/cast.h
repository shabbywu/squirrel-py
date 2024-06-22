#pragma once
#include <type_traits>
#include <concepts>
#include "sqbinding/detail/common/cast.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/pybinding/types/definition.h"



namespace sqbinding {
    namespace python {
        PyValue sqobject_topython(SQObjectPtr& object, detail::VM vm);
        SQObjectPtr pyvalue_tosqobject(PyValue object, detail::VM vm);
        PyValue pyobject_topyvalue(py::object object);
    }

    namespace detail {
        template <> inline
        SQObjectPtr generic_cast(VM vm, PyValue& obj) {
            return python::pyvalue_tosqobject(obj, vm);
        }

        // cast sqobject to pyvalue
        template <> inline
        PyValue generic_cast(VM vm, HSQOBJECT& obj) {
            SQObjectPtr ptr = obj;
            return python::sqobject_topython(ptr, vm);
        }

        // cast sqobjectptr to pyvalue
        template <> inline
        PyValue generic_cast(VM vm, SQObjectPtr& obj) {
            return python::sqobject_topython(obj, vm);
        }
    }
}
