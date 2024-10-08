#pragma once
#include "definition.h"
#include "pyfunction.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqstr.hpp"

namespace py = pybind11;

namespace sqbinding {
namespace python {
class SQPythonObject {
  public:
    py::object _val;
    bool is_class = false;
    // delegate table
    std::shared_ptr<sqbinding::python::Table> _delegate;

    SQPythonObject(py::object object, detail::VM vm)
        : _val(object), _delegate(std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm))) {
        py::type type_ = py::type::of(this->_val);
        if (type_.attr("__name__").cast<std::string>() == "type") {
            is_class = true;
        }
        _delegate->bindFunc(
            "_get",
            python::NativeClosure::Create<python::dynamic_args_function<2>>(
                [this](py::list args) -> PyValue {
                    // (detail::string key) -> PyValue
                    if (is_class) {
                        return this->_val.attr("__dict__").cast<py::dict>().attr("__getitem__")(*args).cast<PyValue>();
                    }
                    return this->_val.attr("__getattribute__")(*args);
                },
                vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_set", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                        [this](py::list args) {
                                            // (detail::string key, PyValue value)
                                            this->_val.attr("__setattr__")(*args);
                                        },
                                        vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_newslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                            [this](py::list args) {
                                                // (detail::string key, PyValue value)
                                                this->_val.attr("__setattr__")(*args);
                                            },
                                            vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_delslot", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                            [this](py::list args) {
                                                // (detail::string key)
                                                this->_val.attr("__delattr__")(*args);
                                            },
                                            vm, python::dynamic_args_function<2>::caller));

        {
            auto closure = python::NativeClosure::Create<python::dynamic_args_function<3>>(
                [this](py::list args) -> PyValue {
                    if (is_class) {
                        auto instance = _val.attr("__new__")(_val);
                        _val.attr("__init__")(instance, *args);
                        return instance.cast<PyValue>();
                    }
                    return this->_val.attr("__call__")(*args).cast<PyValue>();
                },
                vm, python::dynamic_args_function<3>::caller);

            try {
                auto funcName = _val.attr("__name__").cast<py::str>();
                closure.pNativeClosure()->_name = pyvalue_tosqobject(funcName, vm);
            }
            catch (const std::exception &e) {
            }

            _delegate->bindFunc("_call", std::move(closure));
        }

        _delegate->bindFunc("_rawcall", python::NativeClosure::Create<python::dynamic_args_function<2>>(
                                            [this](py::list args) -> PyValue {
                                                return this->_val.attr("__call__")(*args).cast<PyValue>();
                                            },
                                            vm, python::dynamic_args_function<2>::caller));

        _delegate->bindFunc("_typeof", python::NativeClosure::Create<python::dynamic_args_function<3>>(
                                           [this](py::list args) -> PyValue {
                                               py::type type_ = py::type::of(this->_val);
                                               return std::string(type_.attr("__module__").cast<std::string>() + "." +
                                                                  type_.attr("__name__").cast<std::string>());
                                           },
                                           vm, python::dynamic_args_function<3>::caller));
    }

    static SQUserData *Create(py::object object, detail::VM vm) {
        // new userdata to store py::object
        auto result = detail::make_stack_object<SQPythonObject, py::object, detail::VM>(vm, object, vm);
        auto pycontainer = result.first;
        auto ud = result.second;
        ud->SetDelegate(pycontainer->_delegate->pTable());
        ud->_typetag = (void *)PythonTypeTags::TYPE_OBJECT;
        return ud;
    }
};
} // namespace python
} // namespace sqbinding
