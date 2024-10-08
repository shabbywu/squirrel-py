#include "sqbinding/detail/common/errors.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

void register_squirrel_exceptions(py::module_ &m) {
    py::register_exception<sqbinding::index_error>(m, "IndexError", PyExc_IndexError);
    py::register_exception<sqbinding::key_error>(m, "KeyError", PyExc_KeyError);
    py::register_exception<sqbinding::value_error>(m, "ValueError", PyExc_ValueError);
    py::register_exception<sqbinding::stop_iteration>(m, "StopIteration", PyExc_StopIteration);
}
