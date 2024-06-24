#pragma once
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/common/call_setup.h"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/pybinding/common/dynamic_args_function.h"
#include "definition.h"
#include "sqfunction.h"
#include "sqstr.hpp"

namespace py = pybind11;


namespace sqbinding {
    namespace python {
        // PythonNativeCall: wrapper for python function, will not pass squirrel env to python object.
        // this function will be used to call python func in SQVM and return result to SQVM
        static SQInteger PythonNativeCall(HSQUIRRELVM vm) {
            py::gil_scoped_acquire acquire;
            py::function* func;
            sq_getuserdata(vm, -1, (void**)&func, NULL);

            auto vm_ = detail::VM(vm);
            // squirrel 堆栈索引从 1 开始, 且位置 1 是 this(env)
            // 参数从索引 2 开始
            auto args = detail::load_args<2, py::list>::load(vm_);
            PyValue result = (*func)(*args).cast<PyValue>();
            if (std::holds_alternative<py::none>(result)){
                return 0;
            }
            sq_pushobject(vm, pyvalue_tosqobject(result, vm));
            return 1;
        }


        // PythonNativeRawCall: wrapper for python function, will pass squirrel env to python object.
        // this function will be used to call python func in SQVM and return result to SQVM
        static SQInteger PythonNativeRawCall(HSQUIRRELVM vm) {
            py::gil_scoped_acquire acquire;
            py::function* func;
            sq_getuserdata(vm, -1, (void**)&func, NULL);

            auto vm_ = detail::VM(vm);
            // squirrel 堆栈索引从 1 开始, 且位置 1 是 this(env)
            // rawcall 参数从索引 1 开始
            auto args = detail::load_args<1, py::list>::load(vm_);
            PyValue result = (*func)(*args).cast<PyValue>();
            if (std::holds_alternative<py::none>(result)){
                return 0;
            }
            sq_pushobject(vm, pyvalue_tosqobject(result, vm));
            return 1;
        }


        class SQPythonFunction {
        public:
            py::function _val;
            // delegate table
            std::shared_ptr<python::Table> _delegate;

            SQPythonFunction(py::function func, detail::VM vm) {
                this->_val = func;
                _delegate = std::make_shared<python::Table>(python::Table(vm));

                // TODO: 部分固定参数的函数替换成 detail::cpp_function
                _delegate->bindFunc("_get", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) -> PyValue {
                    // detail::string key, PyValue value
                    return this->_val.attr("__getattribute__")(*args);
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_set", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) -> SQBool {
                    // detail::string key, PyValue value
                    this->_val.attr("__setattr__")(*args);
                    return 0;
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_newslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) {
                    // detail::string key, PyValue value
                    this->_val.attr("__setattr__")(*args);
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_delslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) {
                    // detail::string key
                    this->_val.attr("__delattr__")(*args);
                }, vm, python::dynamic_args_function<2>::caller));

                {
                    auto closure = python::NativeClosure::Create<python::dynamic_args_function<3>>(
                    [this](py::list args) -> PyValue {
                        return this->_val.attr("__call__")(*args).cast<PyValue>();
                    }, vm, python::dynamic_args_function<3>::caller);

                    try {
                        auto funcName = _val.attr("__name__").cast<py::str>();
                        closure->pNativeClosure()->_name = pyvalue_tosqobject(funcName, vm);
                    } catch (const std::exception& e) {}

                    _delegate->bindFunc("_call", std::move(closure));
                }

                _delegate->bindFunc("_rawcall", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) -> PyValue {
                    return this->_val.attr("__call__")(*args).cast<PyValue>();
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_typeof", python::NativeClosure::Create<python::dynamic_args_function<3>>(
                [this](py::list args) -> PyValue {
                    py::type type_ = py::type::of(this->_val);
                    return std::string(type_.attr("__module__").cast<std::string>() + "." + type_.attr("__name__").cast<std::string>());
                }, vm, python::dynamic_args_function<3>::caller));
            }

            static SQUserData* Create(py::function func, detail::VM vm) {
                // new userdata to store py::function
                auto result = detail::make_stack_object<SQPythonFunction, py::function, detail::VM>(vm, func, vm);
                auto pycontainer = result.first;
                auto ud = result.second;
                ud->SetDelegate(pycontainer->_delegate->pTable());
                ud->_typetag = (void*)PythonTypeTags::TYPE_FUNCTION;
                return ud;
            }
        };
    }
}
