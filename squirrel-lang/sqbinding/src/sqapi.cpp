#include <pybind11/pybind11.h>
#include <squirrel.h>
#include <sqvm.h>
#include <sqstate.h>
#include <sqtable.h>


namespace py = pybind11;


void register_squirrel_api(py::module_ &m) {
    m.def("sq_pushstring", &sq_pushstring, "pushes a string in the stack", py::arg("vm"), py::arg("s"), py::arg("len"));
    m.def("sq_pushobject", &sq_pushobject, "push an object referenced by an object handler into the stack.", py::arg("vm"), py::arg("obj"));
    m.def("sq_getrefcount", &sq_getrefcount, "returns the number of references of a given object.", py::arg("vm"), py::arg("obj"));
}
