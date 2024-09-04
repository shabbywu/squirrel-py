#pragma once
#include "sqbinding/pybinding/types/definition.h"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/detail/cast.hpp"
#include <concepts>
#include <type_traits>

namespace sqbinding {
namespace python {
PyValue sqobject_topython(SQObjectPtr &object, detail::VM vm);
PyValue sqobject_topython(SQObjectPtr &&object, detail::VM vm);
SQObjectPtr pyvalue_tosqobject(PyValue object, detail::VM vm);
PyValue pyobject_topyvalue(py::object object);
} // namespace python

namespace detail {

// cast PyValue to SQObjectPtr|HSQOBJECT
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, PyValue>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<ToType>, HSQOBJECT>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    return python::pyvalue_tosqobject(from, vm);
}

// cast HSQOBJECT|SQObjectPtr to PyValue
template <typename FromType, typename ToType,
          typename std::enable_if_t<std::is_same_v<std::decay_t<FromType>, SQObjectPtr> ||
                                    std::is_same_v<std::decay_t<FromType>, HSQOBJECT>> * = nullptr,
          typename std::enable_if_t<std::is_same_v<std::decay_t<ToType>, PyValue>> * = nullptr>
static ToType generic_cast(detail::VM vm, FromType &&from) {
#ifdef TRACE_OBJECT_CAST
    std::cout << "[TRACING] cast " << typeid(decltype(from)).name() << " to " << typeid(ToType).name() << std::endl;
#endif
    typename std::remove_reference_t<FromType> copy = from;
    return python::sqobject_topython(copy, vm);
}


// cast py::args to SQObjectPtr|HSQOBJECT




} // namespace detail
} // namespace sqbinding
