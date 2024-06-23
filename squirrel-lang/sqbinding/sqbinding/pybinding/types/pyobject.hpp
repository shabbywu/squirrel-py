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
            std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
            std::map<std::string, std::shared_ptr<sqbinding::python::NativeClosure>> nativeclosure_handlers;

            SQPythonObject(py::object object, detail::VM vm) {
                this->_val = object;

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
                cppfunction_handlers["_typeof"] = std::make_shared<py::cpp_function>([this]() -> std::string {
                    py::type type_ = py::type::of(this->_val);
                    return std::string(type_.attr("__module__").cast<std::string>() + "." + type_.attr("__name__").cast<std::string>());
                });

                for(auto& [ k, v ]: cppfunction_handlers) {
                    nativeclosure_handlers[k] = std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure{v, vm, &PythonNativeCall});
                }

                _delegate = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm));
                for(auto pair: nativeclosure_handlers) {
                    _delegate->bindFunc(pair.first, pair.second);
                }
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
