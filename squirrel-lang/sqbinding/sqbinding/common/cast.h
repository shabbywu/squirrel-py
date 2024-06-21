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
}
