#pragma once

#include <squirrel.h>

#include "definition.h"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/types/sqfunction.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/common/dynamic_args_function.h"
#include "sqbinding/pybinding/common/stack_operation.h"

namespace sqbinding {
namespace python {
class Closure : public detail::Closure<PyValue(py::args)> {
  public:
    Closure(::SQClosure *pClosure, detail::VM vm) : detail::Closure<PyValue(py::args)>(pClosure, vm) {};

  public:
    void bind_this_if_need(PyValue &v);
    PyValue operator()(py::args args) {
        detail::VM &vm = holder->GetVM();
        detail::stack_guard stack_guard(vm);
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args);
        } else {
            sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args);
        }
        return sq_call<PyValue>(vm, stack_guard.offset() - 1);
    }
    // Python API
    PyValue get(PyValue key) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        auto v = detail::Closure<PyValue(py::args)>::get<PyValue, PyValue>(std::forward<PyValue>(key));
        bind_this_if_need(v);
        return v;
    }

    PyValue __call__(py::args args) {
        return this->operator()(args);
    }
    std::string __str__() {
        return to_string();
    }
    std::string __repr__() {
        return "Closure(" + to_string() + ")";
    }
};
} // namespace python
} // namespace sqbinding

namespace sqbinding {
namespace python {
using BaseNativeClosure = detail::NativeClosure<PyValue(py::args)>;
class NativeClosure : public BaseNativeClosure {
  public:
    NativeClosure(::SQNativeClosure *pNativeClosure, detail::VM vm) : BaseNativeClosure(pNativeClosure, vm) {};
    NativeClosure(std::shared_ptr<py::function> func, detail::VM vm, SQFUNCTION caller)
        : BaseNativeClosure(::SQNativeClosure::Create(_ss(*vm), caller, 1), vm) {
        // TODO: 重构 new userdata 的方式
        pNativeClosure()->_nparamscheck = 0;
        SQUserPointer ptr = sq_newuserdata(*vm, sizeof(py::function));
        std::memcpy(ptr, func.get(), sizeof(py::function));
        pNativeClosure()->_outervalues[0] = (*vm)->PopGet();
    }

  public:
    void bind_this_if_need(PyValue &v);
    PyValue operator()(py::args args) {
        detail::VM &vm = holder->GetVM();
        detail::stack_guard stack_guard(vm);
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args);
        } else {
            sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args);
        }
        return sq_call<PyValue>(vm, stack_guard.offset() - 1);
    }
    // Python API
    PyValue get(PyValue key) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        auto v = BaseNativeClosure::get<PyValue, PyValue>(std::forward<PyValue>(key));
        bind_this_if_need(v);
        return v;
    }
    PyValue __call__(py::args args) {
        return this->operator()(args);
    }
    std::string __str__() {
        return to_string();
    }
    std::string __repr__() {
        return "NativeClosure(" + to_string() + ")";
    }

  public:
    template <class Wrapper, class Func> static NativeClosure Create(Func &&func, detail::VM vm, SQFUNCTION caller) {
        auto pair = detail::make_stack_object<Wrapper>(vm, func);
        NativeClosure closure(SQNativeClosure::Create(_ss(*vm), caller, 1), vm);
        closure.pNativeClosure()->_outervalues[0] = pair.second;
        closure.pNativeClosure()->_nparamscheck = 0;
        return closure;
    }
};
} // namespace python
} // namespace sqbinding
