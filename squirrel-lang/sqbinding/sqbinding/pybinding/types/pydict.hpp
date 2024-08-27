#pragma once
#include "definition.h"
#include "pyfunction.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/pybinding/common/cast.h"

namespace py = pybind11;

namespace sqbinding {
namespace python {
class SQPythonDict {
  public:
    py::dict _val;
    // delegate table
    std::shared_ptr<sqbinding::python::Table> _delegate;

    SQPythonDict(py::dict dict, detail::VM vm)
        : _val(dict), _delegate(std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm))) {

        _delegate->bindFunc("_get", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                        [this](py::list args) -> PyValue {
                                            // (detail::string key) -> PyValue
                                            return this->_val.attr("__getitem__")(*args);
                                        },
                                        vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_set", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                        [this](py::list args) {
                                            // (detail::string key, PyValue value)
                                            this->_val.attr("__setitem__")(*args);
                                        },
                                        vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_newslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                            [this](py::list args) {
                                                // (detail::string key, PyValue value)
                                                this->_val.attr("__setitem__")(*args);
                                            },
                                            vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_delslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                            [this](py::list args) {
                                                // (detail::string key)
                                                this->_val.attr("__delitem__")(*args);
                                            },
                                            vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("pop", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                       [this](py::list args) -> PyValue {
                                           // (detail::string key, PyValue value) -> PyValue
                                           return this->_val.attr("pop")(*args);
                                       },
                                       vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("len", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                       [this](py::list args) -> PyValue {
                                           // ()  -> PyValue value
                                           return py::int_(this->_val.size());
                                       },
                                       vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("clear", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                         [this](py::list args) {
                                             // ()
                                             this->_val.clear();
                                         },
                                         vm, python::dynamic_args_function<2>::caller));
    }

    static SQUserData *Create(py::dict dict, detail::VM vm) {
        // new userdata to store py::dict
        auto result = detail::make_stack_object<SQPythonDict, py::dict, detail::VM>(vm, dict, vm);
        auto pycontainer = result.first;
        auto ud = result.second;
        ud->SetDelegate(pycontainer->_delegate->pTable());
        ud->_typetag = (void *)PythonTypeTags::TYPE_DICT;
        return ud;
    }
};
} // namespace python
} // namespace sqbinding
