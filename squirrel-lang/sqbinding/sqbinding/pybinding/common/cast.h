#pragma once
#include <type_traits>
#include "sqbinding/detail/common/cast.hpp"
#include "sqbinding/pybinding/types/definition.h"


namespace sqbinding {
    namespace python {
        PyValue sqobject_topython(SQObjectPtr& object, HSQUIRRELVM vm);
        SQObjectPtr pyvalue_tosqobject(PyValue object, HSQUIRRELVM vm);
        PyValue pyobject_topyvalue(py::object object);
    }

    namespace detail {
        template <> inline
        SQObjectPtr generic_cast(HSQUIRRELVM vm, PyValue& obj) {
            return python::pyvalue_tosqobject(obj, vm);
        }

        // cast sqobject to pyvalue
        template <> inline
        PyValue generic_cast(HSQUIRRELVM vm, HSQOBJECT& obj) {
            SQObjectPtr ptr = obj;
            return python::sqobject_topython(ptr, vm);
        }

        // cast sqobjectptr to pyvalue
        template <> inline
        PyValue generic_cast(HSQUIRRELVM vm, SQObjectPtr& obj) {
            return python::sqobject_topython(obj, vm);
        }
    }
}
