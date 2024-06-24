#pragma once
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "definition.h"
#include "sqstr.hpp"
#include "pyfunction.hpp"


namespace py = pybind11;


namespace sqbinding {
    namespace python {
        class SQPythonObject {
        public:
            py::object _val;
            // delegate table
            std::shared_ptr<sqbinding::python::Table> _delegate;

            SQPythonObject(py::object object, detail::VM vm) {
                this->_val = object;
                _delegate = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm));

                _delegate->bindFunc("_get", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) -> PyValue {
                    // (detail::string key) -> PyValue
                    return this->_val.attr("__getattribute__")(*args);
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_set", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) {
                    // (detail::string key, PyValue value)
                    this->_val.attr("__setattr__")(*args);
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_newslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) {
                    // (detail::string key, PyValue value)
                    this->_val.attr("__setattr__")(*args);
                }, vm, python::dynamic_args_function<2>::caller));

                _delegate->bindFunc("_delslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) {
                    // (detail::string key)
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

            static SQUserData* Create(py::object object, detail::VM vm) {
                // new userdata to store py::object
                auto result = detail::make_stack_object<SQPythonObject, py::object, detail::VM>(vm, object, vm);
                auto pycontainer = result.first;
                auto ud = result.second;
                ud->SetDelegate(pycontainer->_delegate->pTable());
                ud->_typetag = (void*)PythonTypeTags::TYPE_OBJECT;
                return ud;
            }
        };
    }
}
