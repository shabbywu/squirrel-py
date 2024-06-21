#pragma once
#include "sqbinding/types/definition.h"


namespace sqbinding {
    namespace detail {
        std::string sqobject_to_string(SQObjectPtr&);
    }

    namespace python {
        PyValue sqobject_topython(SQObjectPtr& object, HSQUIRRELVM vm);
        SQObjectPtr pyvalue_tosqobject(PyValue object, HSQUIRRELVM vm);
        PyValue pyobject_topyvalue(py::object object);
    }

    namespace detail {
        template <class FromType, class ToType> inline
        ToType generic_cast(HSQUIRRELVM vm, FromType& obj);

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
