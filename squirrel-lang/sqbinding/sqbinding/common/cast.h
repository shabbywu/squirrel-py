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
        template <class Type> inline
        Type generic_cast(HSQUIRRELVM vm, HSQOBJECT& obj);

        template <> inline
        PyValue generic_cast(HSQUIRRELVM vm, HSQOBJECT& obj) {
            SQObjectPtr ptr = obj;
            return python::sqobject_topython(ptr, vm);
        }
    }
}
