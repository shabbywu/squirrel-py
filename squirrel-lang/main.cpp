#include <pybind11/pybind11.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


namespace py = pybind11;


void register_squirrel_type(py::module_ &m);
void register_squirrel_api(py::module_ &m);
void register_squirrel_vm(py::module_ &m);


PYBIND11_MODULE(_squirrel, m) {
    register_squirrel_vm(m);

    py::module mTypes = m.def_submodule("_types", "types for sq");
    register_squirrel_type(mTypes);
    
    py::module mLowLevel = m.def_submodule("sqapi", "low level api for sq");
    register_squirrel_api(mLowLevel);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif

    m.attr("__author__") = "shabbywu<shabbywu@qq.com>";
}
