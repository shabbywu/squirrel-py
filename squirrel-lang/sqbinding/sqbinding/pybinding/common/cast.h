#pragma once
#include "sqbinding/detail/common/cast_impl.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/pybinding/types/definition.h"
#include <concepts>
#include <type_traits>

namespace sqbinding {
namespace python {
PyValue sqobject_topython(SQObjectPtr &object, detail::VM vm);
SQObjectPtr pyvalue_tosqobject(PyValue object, detail::VM vm);
PyValue pyobject_topyvalue(py::object object);
} // namespace python

namespace detail {
template <typename FromType>
class GenericCast<SQObjectPtr(FromType), typename std::enable_if_t<std::is_convertible_v<FromType, PyValue>>> {
  public:
    static SQObjectPtr cast(VM vm, std::remove_reference_t<FromType> &obj) {
        return python::pyvalue_tosqobject(obj, vm);
    }

    static SQObjectPtr cast(VM vm, std::remove_reference_t<FromType> &&obj) {
        return python::pyvalue_tosqobject(obj, vm);
    }
};

// cast sqobject to pyvalue
template <> class GenericCast<PyValue(HSQOBJECT &)> {
  public:
    static PyValue cast(VM vm, HSQOBJECT &obj) {
        SQObjectPtr ptr = obj;
        return python::sqobject_topython(ptr, vm);
    }
};

template <> class GenericCast<PyValue(HSQOBJECT &&)> {
  public:
    static PyValue cast(VM vm, HSQOBJECT &&obj) {
        SQObjectPtr ptr = obj;
        return python::sqobject_topython(ptr, vm);
    }
};

// cast sqobjectptr to pyvalue
template <> class GenericCast<PyValue(SQObjectPtr &)> {
  public:
    static PyValue cast(VM vm, SQObjectPtr &ptr) {
        return python::sqobject_topython(ptr, vm);
    }
};

template <> class GenericCast<PyValue(SQObjectPtr &&)> {
  public:
    static PyValue cast(VM vm, SQObjectPtr &&ptr) {
        return python::sqobject_topython(ptr, vm);
    }
};
} // namespace detail
} // namespace sqbinding
