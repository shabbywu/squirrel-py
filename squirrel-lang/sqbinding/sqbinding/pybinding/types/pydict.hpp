#pragma once
#include "sqbinding/pybinding/common/cast.h"
#include "definition.h"
#include "pyfunction.hpp"

namespace py = pybind11;

namespace sqbinding {
    namespace python {
        class SQPythonDict {
        public:
            detail::VM vm;
            py::dict _val;
            // delegate table
            std::shared_ptr<sqbinding::python::Table> _delegate;
            std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
            std::map<std::string, std::shared_ptr<sqbinding::python::NativeClosure>> nativeclosure_handlers;

            SQPythonDict(py::dict dict, detail::VM vm) {
                this->vm = vm;
                this->_val = dict;

                cppfunction_handlers["_get"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key) -> PyValue {
                    return this->_val[py::str(key)].cast<PyValue>();
                });
                cppfunction_handlers["_set"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value) {
                    this->_val.attr("__setitem__")(key, value);
                });
                cppfunction_handlers["_newslot"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value){
                    this->_val.attr("__setitem__")(key, value);
                });
                cppfunction_handlers["_delslot"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key) {
                    this->_val.attr("__delitem__")(key);
                });

                cppfunction_handlers["pop"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value) -> PyValue {
                    return this->_val.attr("pop")(key, value);
                });
                cppfunction_handlers["len"] = std::make_shared<py::cpp_function>([this]() -> PyValue {
                    return py::int_(this->_val.size());
                });
                cppfunction_handlers["clear"] = std::make_shared<py::cpp_function>([this]() {
                    this->_val.clear();
                });

                for(const auto& [ k, v ]: cppfunction_handlers) {
                    nativeclosure_handlers[k] = std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure{v, vm, &PythonNativeCall});
                }

                _delegate = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm));
                for(auto pair: nativeclosure_handlers) {
                    _delegate->bindFunc(pair.first, pair.second);
                }
            }

            static SQUserData* Create(py::dict dict, detail::VM vm) {
                // new userdata to store py::dict
                auto result = detail::make_stack_object<SQPythonDict, py::dict, detail::VM>(vm, dict, vm);
                auto pycontainer = result.first;
                auto ud = result.second;
                ud->SetDelegate(pycontainer->_delegate->pTable());
                ud->_typetag = (void*)PythonTypeTags::TYPE_DICT;
                return ud;
            }
        };
    }
}
