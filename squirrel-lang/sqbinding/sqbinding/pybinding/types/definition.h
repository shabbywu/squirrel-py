#ifndef _SQBINDING_DEFINITION_H_
#define _SQBINDING_DEFINITION_H_
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <map>
#include <string>
#include <variant>

#include "sqbinding/detail/sqdefinition.hpp"

namespace sqbinding {
namespace python {
class SQPythonDict;
class SQPythonList;
class SQPythonObject;

class ObjectPtr;
class Closure;
class NativeClosure;
class Table;
class TableIterator;
class Array;
class ArrayIterator;
class Instance;
class Class;
class String;
} // namespace python
} // namespace sqbinding

namespace py = pybind11;

typedef std::variant<py::none, std::shared_ptr<sqbinding::python::String>, std::shared_ptr<sqbinding::python::Array>,
                     std::shared_ptr<sqbinding::python::Table>, std::shared_ptr<sqbinding::python::Closure>,
                     std::shared_ptr<sqbinding::python::NativeClosure>, std::shared_ptr<sqbinding::python::Class>,
                     std::shared_ptr<sqbinding::python::Instance>, std::shared_ptr<sqbinding::python::ArrayIterator>,
                     std::shared_ptr<sqbinding::python::TableIterator>, py::int_, py::float_, py::bool_, std::string,
                     py::list, py::dict, py::function, py::type, py::object>
    PyValue;

enum class PythonTypeTags {
    TYPE_DICT = 0b01,
    TYPE_LIST = 0b10,
    TYPE_FUNCTION = 0b100,
    TYPE_OBJECT = 0b1000,
};
#endif
