#pragma once

#include "definition.h"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/types/sqarray.hpp"
#include "sqbinding/pybinding/common/cast.h"

namespace sqbinding {
namespace python {
class ArrayIterator : public detail::ArrayIterator {
  public:
    using detail::ArrayIterator::ArrayIterator;
    PyValue operator*() const {
        SQObjectPtr ret = detail::ArrayIterator::operator*();
        return detail::GenericCast<PyValue(SQObjectPtr &)>::cast(holder->GetVM(), ret);
    }
    PyValue __next__() {
        if (idx < 0) {
            throw py::stop_iteration();
        }
        PyValue result;
        try {
            result = **this;
        }
        catch (const sqbinding::index_error &e) {
            throw py::stop_iteration();
        }
        idx++;
        return result;
    }
};
class Array : public detail::Array, public std::enable_shared_from_this<Array> {
  public:
    // create a array in vm stack
    Array(detail::VM vm) : detail::Array(SQArray::Create(_ss(*vm), 4), vm) {
    }
    Array(SQArray *pArray, detail::VM vm) : detail::Array(pArray, vm) {
    }

    // Python Interface
    PyValue __getitem__(int idx) {
        return get<int, PyValue>(idx);
    }
    PyValue __setitem__(int idx, PyValue val) {
        set<int, PyValue>(idx, val);
        return val;
    }
    ArrayIterator __iter__() {
        return ArrayIterator(holder, 0);
    }
    SQInteger __len__() {
        return size();
    }

    std::string __str__() {
        return to_string();
    }

    std::string __repr__() {
        return "SQArray(" + to_string() + ")";
    }
};
} // namespace python
} // namespace sqbinding
