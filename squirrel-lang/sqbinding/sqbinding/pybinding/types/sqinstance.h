#pragma once

#include "definition.h"
#include "pydict.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/types/sqinstance.hpp"
#include "sqbinding/pybinding/common/cast.h"

namespace sqbinding {
namespace python {
class Instance : public detail::Instance, std::enable_shared_from_this<Instance> {
  public:
    // link to a existed pInstance in vm stack
    Instance(::SQInstance *pInstance, detail::VM vm) : detail::Instance(pInstance, vm) {};
  public:
    // bindFunc to current table
    // FIXME: 让 bindfunc 只支持绑定 python 方法?
    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        // TODO: 实装支持 withenv
        set<std::string, Func>(std::forward<std::string>(funcname), std::forward<Func>(func));
    }
  public:
    void bind_this_if_need(PyValue &v);
    // Python API
    PyValue get(PyValue key) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        PyValue v = detail::Instance::get<PyValue, PyValue>(std::forward<PyValue>(key));
        bind_this_if_need(v);
        return v;
    }
    // Python Interface
    PyValue __getitem__(PyValue &key) {
        return get(key);
    }
    PyValue __setitem__(PyValue key, PyValue val) {
        set<PyValue, PyValue>(std::forward<PyValue>(key), std::forward<PyValue>(val));
        return val;
    }
    py::list keys() {
        detail::VM &vm = holder->vm;
        SQInteger idx = 0;
        py::list keys;
        auto table = pInstance()->_class->_members;
        while (idx < table->_numofnodes) {
            auto n = &table->_nodes[idx++];
            if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
                keys.append(detail::generic_cast<SQObjectPtr, PyValue>(vm, std::forward<SQObjectPtr>(n->key)));
            }
        }
        return keys;
    }

    std::string __str__() {
        return string_format("OT_INSTANCE: [addr={%p}, ref=%d]", pInstance(), getRefCount());
    }

    std::string __repr__() {
        return "SQInstance(" + __str__() + ")";
    }
};
} // namespace python
} // namespace sqbinding
