#pragma once

#include "definition.h"
#include "pyfunction.hpp"
#include "sqbinding/pybinding/common/cast.h"

namespace py = pybind11;


namespace sqbinding {
    namespace python {
        class SQPythonList {
        public:
            detail::VM vm;
            py::list _val;
            // delegate table
            std::shared_ptr<sqbinding::python::Table> _delegate;
            std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
            std::map<std::string, std::shared_ptr<sqbinding::python::NativeClosure>> nativeclosure_handlers;

            SQPythonList(py::list list, detail::VM vm) {
                this->vm = vm;
                this->_val = list;

                cppfunction_handlers["_get"] = std::make_shared<py::cpp_function>([this](py::int_ key) -> PyValue {
                    return this->_val[key].cast<PyValue>();
                });
                cppfunction_handlers["_set"] = std::make_shared<py::cpp_function>([this](py::int_ key, PyValue value){
                    this->_val.attr("__setitem__")(key, value);
                });
                cppfunction_handlers["_newslot"] = std::make_shared<py::cpp_function>([this](py::int_ key, PyValue value){
                    this->_val.attr("__setitem__")(key, value);
                });
                cppfunction_handlers["_delslot"] = std::make_shared<py::cpp_function>([this](py::int_ key) {
                    this->_val.attr("__delitem__")(key);
                });

                cppfunction_handlers["append"] = std::make_shared<py::cpp_function>([this](PyValue value){
                    this->_val.attr("append")(value);
                });
                cppfunction_handlers["pop"] = std::make_shared<py::cpp_function>([this](PyValue value) -> PyValue {
                    return this->_val.attr("pop")(value);
                });
                cppfunction_handlers["len"] = std::make_shared<py::cpp_function>([this]() -> PyValue {
                    return this->_val.attr("__len__")();
                });

                for(const auto& [ k, v ]: cppfunction_handlers) {
                    nativeclosure_handlers[k] = std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure{v, vm, &PythonNativeCall});
                }

                _delegate = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm));
                for(auto pair: nativeclosure_handlers) {
                    _delegate->bindFunc(pair.first, pair.second);
                }
            }

            static SQUserData* Create(py::list list, detail::VM vm) {
                // new userdata to store py::list
                auto result = detail::make_stack_object<SQPythonList, py::list, detail::VM>(vm, list, vm);
                auto pycontainer = result.first;
                auto ud = result.second;
                ud->SetDelegate(pycontainer->_delegate->pTable());
                ud->_typetag = (void*)PythonTypeTags::TYPE_LIST;
                return ud;
            }
        };
    }
}
