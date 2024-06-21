#include "sqbinding/types/definition.h"
#include "sqbinding/types/container.h"

namespace py = pybind11;


void register_squirrel_type(py::module_ &m) {
    py::class_<SQVM, std::shared_ptr<SQVM>>(m, "HSQUIRRELVM")
        .def_readonly("top", &SQVM::_top)
        .def_readonly("stackbase", &SQVM::_stackbase)
        .def_readonly("callsstacksize", &SQVM::_callsstacksize)
        .def_readonly("alloccallsstacksize", &SQVM::_alloccallsstacksize)
        .def_readonly("suspended", &SQVM::_suspended)
        .def_readonly("suspended_root", &SQVM::_suspended_root);


    py::enum_<tagSQObjectType>(m, "tagSQObjectType")
        .value("OT_NULL", tagSQObjectType::OT_NULL)
        .value("OT_INTEGER", tagSQObjectType::OT_INTEGER)
        .value("OT_FLOAT", tagSQObjectType::OT_FLOAT)
        .value("OT_BOOL", tagSQObjectType::OT_BOOL)
        .value("OT_STRING", tagSQObjectType::OT_STRING)
        .value("OT_TABLE", tagSQObjectType::OT_TABLE)
        .value("OT_ARRAY", tagSQObjectType::OT_ARRAY)
        .value("OT_USERDATA", tagSQObjectType::OT_USERDATA)
        .value("OT_CLOSURE", tagSQObjectType::OT_CLOSURE)
        .value("OT_NATIVECLOSURE", tagSQObjectType::OT_NATIVECLOSURE)
        .value("OT_GENERATOR", tagSQObjectType::OT_GENERATOR)
        .value("OT_USERPOINTER", tagSQObjectType::OT_USERPOINTER)
        .value("OT_THREAD", tagSQObjectType::OT_THREAD)
        .value("OT_FUNCPROTO", tagSQObjectType::OT_FUNCPROTO)
        .value("OT_CLASS", tagSQObjectType::OT_CLASS)
        .value("OT_INSTANCE", tagSQObjectType::OT_INSTANCE)
        .value("OT_WEAKREF", tagSQObjectType::OT_WEAKREF)
        .value("OT_OUTER", tagSQObjectType::OT_OUTER);

    py::class_<tagSQObjectValue, std::shared_ptr<tagSQObjectValue>>(m, "tagSQObjectValue");

    py::class_<ArrayIterator, std::shared_ptr<ArrayIterator>>(m, "ArrayIterator")
    .def("__next__", &ArrayIterator::__next__)
    ;

    py::class_<TableIterator, std::shared_ptr<TableIterator>>(m, "TableIterator")
    .def("__next__", &TableIterator::__next__)
    ;

    {
        py::class_<sqbinding::python::Table, std::shared_ptr<sqbinding::python::Table>>(m, "SQTable")
        .def(py::init([](HSQUIRRELVM vm) { return std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm)); }))
        .def("__iter__", &sqbinding::python::Table::__iter__, py::return_value_policy::reference_internal)
        .def("__getitem__", &sqbinding::python::Table::__getitem__, py::arg("key"), py::return_value_policy::move)
        .def("__setitem__", &sqbinding::python::Table::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
        .def("__delitem__", &sqbinding::python::Table::__delitem__, py::arg("key"))
        .def("__getattr__", &sqbinding::python::Table::__getitem__, py::arg("key"), py::return_value_policy::move)
        .def("__setattr__", &sqbinding::python::Table::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
        .def("__delattr__", &sqbinding::python::Table::__delitem__, py::arg("key"))
        .def("keys", &sqbinding::python::Table::keys, py::return_value_policy::take_ownership)
        .def("__len__", &sqbinding::python::Table::__len__)
        .def("bindfunc", &sqbinding::python::Table::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("get_ref_count", &sqbinding::python::Table::getRefCount)
        ;
    }

    py::class_<_SQClass_, std::shared_ptr<_SQClass_>>(m, "SQClass")
    .def("__getitem__", &_SQClass_::__getitem__, py::arg("key"), py::return_value_policy::move)
    .def("__setitem__", &_SQClass_::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("__getattr__", &_SQClass_::__getitem__, py::arg("key"), py::return_value_policy::move)
    .def("__setattr__", &_SQClass_::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("keys", &_SQClass_::keys, py::return_value_policy::take_ownership)
    .def("bindfunc", &_SQClass_::bindFunc, py::arg("funcname"), py::arg("func"))
    ;

    py::class_<_SQInstance_, std::shared_ptr<_SQInstance_>>(m, "SQInstance")
    .def("__getitem__", &_SQInstance_::__getitem__, py::arg("key"), py::return_value_policy::move)
    .def("__setitem__", &_SQInstance_::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("__getattr__", &_SQInstance_::__getitem__, py::arg("key"), py::return_value_policy::move)
    .def("__setattr__", &_SQInstance_::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("keys", &_SQInstance_::keys, py::return_value_policy::take_ownership)
    .def("bindfunc", &_SQInstance_::bindFunc, py::arg("funcname"), py::arg("func"))
    .def("__str__", &_SQInstance_::__str__)
    .def("__repr__", &_SQInstance_::__repr__)
    ;

    py::class_<_SQString_, std::shared_ptr<_SQString_>>(m, "SQString")
    .def_property_readonly("value", &_SQString_::value)
    .def("__len__", &_SQString_::__len__)
    .def("__str__", &_SQString_::__str__)
    .def("__repr__", &_SQString_::__repr__)
    ;

    {
        py::class_<sqbinding::python::Closure, std::shared_ptr<sqbinding::python::Closure>>(m, "SQClosure")
        .def("__call__", &sqbinding::python::Closure::__call__, py::return_value_policy::move)
        .def("__str__", &sqbinding::python::Closure::__str__)
        .def("__repr__", &sqbinding::python::Closure::__repr__)
        .def("__getattr__", &sqbinding::python::Closure::get, py::arg("key"), py::return_value_policy::move)
        ;
    }


    py::class_<sqbinding::python::NativeClosure, std::shared_ptr<sqbinding::python::NativeClosure>>(m, "SQNativeClosure")
    .def("__call__", &sqbinding::python::NativeClosure::__call__, py::return_value_policy::move)
    .def("__str__", &sqbinding::python::NativeClosure::__str__)
    .def("__repr__", &sqbinding::python::NativeClosure::__repr__)
    .def("__getattr__", &sqbinding::python::NativeClosure::get, py::arg("key"), py::return_value_policy::move)
    .def("clone", [](std::shared_ptr<sqbinding::python::NativeClosure> self) -> std::shared_ptr<sqbinding::python::NativeClosure> {
        return std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure(self->pNativeClosure()->Clone(), self->holder->vm));
    }, py::return_value_policy::move)
    ;

    py::class_<sqbinding::python::Array, std::shared_ptr<sqbinding::python::Array>>(m, "SQArray")
    .def(py::init([](HSQUIRRELVM vm) { return std::make_shared<sqbinding::python::Array>(sqbinding::python::Array(vm)); }))
    .def("__iter__", &sqbinding::python::Array::__iter__, py::return_value_policy::reference_internal)
    .def("__getitem__", &sqbinding::python::Array::__getitem__, py::arg("idx"), py::return_value_policy::move)
    .def("__setitem__", &sqbinding::python::Array::__setitem__, py::arg("idx"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("append", &sqbinding::python::Array::append, py::arg("val"), py::keep_alive<1, 2>())
    .def("pop", &sqbinding::python::Array::pop, py::return_value_policy::reference_internal)
    .def("__len__", &sqbinding::python::Array::__len__)
    ;

    py::class_<SQObjectPtr>(m, "__SQObjectPtr");
    py::class_<_SQObjectPtr_, std::shared_ptr<_SQObjectPtr_>>(m, "SQObjectPtr")
    .def_property_readonly("type", &_SQObjectPtr_::type)
    .def_property("value",
        //py::cpp_function(, py::return_value_policy::automatic),
        &_SQObjectPtr_::to_python,
        &_SQObjectPtr_::from_python)
    .def("__str__", &_SQObjectPtr_::__str__)
    .def("__repr__", &_SQObjectPtr_::__repr__)
    .def_property_readonly("refcount", [](_SQObjectPtr_& self) {
        return sq_getrefcount(self.vm, &self.obj);
    })
    ;
}
