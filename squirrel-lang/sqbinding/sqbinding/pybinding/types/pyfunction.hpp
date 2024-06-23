#pragma once
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/common/call_setup.h"
#include "sqbinding/detail/types/sqvm.hpp"
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
            auto args = detail::load_args<2, py::list(py::list)>::load([](py::list args) -> py::list {return args;}, vm_);
            PyValue result = (*func)(*args()).cast<PyValue>();
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
            auto args = detail::load_args<1, py::list(py::list)>::load([](py::list args) -> py::list {return args;}, vm_);
            PyValue result = (*func)(*args()).cast<PyValue>();
            if (std::holds_alternative<py::none>(result)){
                return 0;
            }
            sq_pushobject(vm, pyvalue_tosqobject(result, vm));
            return 1;
        }


        class SQPythonFunction {
        public:
            detail::VM vm;
            py::function _val;
            // delegate table
            std::shared_ptr<sqbinding::python::Table> _delegate;
            std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
            std::map<std::string, std::shared_ptr<sqbinding::python::NativeClosure>> nativeclosure_handlers;

            SQPythonFunction(py::function func, detail::VM vm) {
                this->vm = vm;
                this->_val = func;

                cppfunction_handlers["_get"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key) -> PyValue {
                    return this->_val.attr("__getattribute__")(key).cast<PyValue>();
                });
                cppfunction_handlers["_set"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value) -> SQBool {
                    this->_val.attr("__setattr__")(key, value);
                    return 0;
                });
                cppfunction_handlers["_newslot"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value){
                    this->_val.attr("__setattr__")(key, value);
                });
                cppfunction_handlers["_delslot"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key) {
                    this->_val.attr("__delattr__")(key);
                });

                cppfunction_handlers["_call"] = std::make_shared<py::cpp_function>([this](PyValue env, py::args args) -> PyValue {
                    return this->_val.attr("__call__")(*args).cast<PyValue>();
                });
                cppfunction_handlers["_rawcall"] = std::make_shared<py::cpp_function>([this](PyValue env, py::args args) -> PyValue {
                    return this->_val.attr("__call__")(*args).cast<PyValue>();
                });

                cppfunction_handlers["_typeof"] = std::make_shared<py::cpp_function>([this]() -> std::string {
                    py::type type_ = py::type::of(this->_val);
                    return std::string(type_.attr("__module__").cast<std::string>() + "." + type_.attr("__name__").cast<std::string>());
                });

                for(auto& [ k, func ]: cppfunction_handlers) {
                    auto withenv = k == "_rawcall";
                    nativeclosure_handlers[k] = std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure{func, vm, withenv? &PythonNativeRawCall: &PythonNativeCall});
                }

                try
                {
                    auto _call = nativeclosure_handlers["_call"];
                    auto funcName = _val.attr("__name__").cast<py::str>();
                    _call->pNativeClosure()->_name = pyvalue_tosqobject(funcName, vm);
                }
                catch(const std::exception& e)
                {

                }

                _delegate = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm));
                for(auto pair: nativeclosure_handlers) {
                    _delegate->bindFunc(pair.first, pair.second);
                }
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
