#pragma once
#include <type_traits>
#include <concepts>
#include "sqbinding/detail/common/cast_impl.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/pybinding/types/definition.h"



namespace sqbinding {
    namespace python {
        PyValue sqobject_topython(SQObjectPtr& object, detail::VM vm);
        SQObjectPtr pyvalue_tosqobject(PyValue object, detail::VM vm);
        PyValue pyobject_topyvalue(py::object object);
    }

    namespace detail {
        template <typename FromType> requires (std::is_convertible_v<FromType, PyValue>)
        class GenericCast<SQObjectPtr(FromType)> {
            public:
            static SQObjectPtr cast(VM vm, FromType& obj) {
                return python::pyvalue_tosqobject(obj, vm);
            }
        };

        template <typename FromType> requires (std::is_convertible_v<FromType, PyValue>)
        class GenericCast<SQObjectPtr(FromType&)> {
            public:
            static SQObjectPtr cast(VM vm, FromType& obj) {
                return python::pyvalue_tosqobject(obj, vm);
            }
        };

        // cast sqobject to pyvalue
        template <>
        class GenericCast<PyValue(HSQOBJECT&)> {
            public:
            static PyValue cast(VM vm, HSQOBJECT& obj) {
                SQObjectPtr ptr = obj;
                return python::sqobject_topython(ptr, vm);
            }
        };

        template <>
        class GenericCast<PyValue(HSQOBJECT&&)> {
            public:
            static PyValue cast(VM vm, HSQOBJECT&& obj) {
                SQObjectPtr ptr = obj;
                return python::sqobject_topython(ptr, vm);
            }
        };

        // cast sqobjectptr to pyvalue
        template <>
        class GenericCast<PyValue(SQObjectPtr&)> {
            public:
            static PyValue cast(VM vm, SQObjectPtr& ptr) {
                return python::sqobject_topython(ptr, vm);
            }
        };

        template <>
        class GenericCast<PyValue(SQObjectPtr&&)> {
            public:
            static PyValue cast(VM vm, SQObjectPtr&& ptr) {
                return python::sqobject_topython(ptr, vm);
            }
        };
    }
}
