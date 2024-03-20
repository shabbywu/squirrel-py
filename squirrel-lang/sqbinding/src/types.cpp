#include "types/definition.h"
#include "types/container.h"

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

    py::class_<_SQTable_, std::shared_ptr<_SQTable_>>(m, "SQTable")
    .def(py::init([](HSQUIRRELVM vm) { return std::make_shared<_SQTable_>(_SQTable_(vm)); }))
    .def("__iter__", &_SQTable_::__iter__, py::return_value_policy::reference_internal)
    .def("__getitem__", &_SQTable_::__getitem__, py::arg("key"), py::return_value_policy::move)
    .def("__setitem__", &_SQTable_::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("__delitem__", &_SQTable_::__delitem__, py::arg("key"))
    .def("__getattr__", &_SQTable_::__getitem__, py::arg("key"), py::return_value_policy::move)
    .def("__setattr__", &_SQTable_::__setitem__, py::arg("key"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("__delattr__", &_SQTable_::__delitem__, py::arg("key"))
    .def("keys", &_SQTable_::keys, py::return_value_policy::take_ownership)
    .def("__len__", &_SQTable_::__len__)
    .def("bindfunc", &_SQTable_::bindFunc, py::arg("funcname"), py::arg("func"))
    .def("get_ref_count", &_SQTable_::getRefCount)
    ;

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

    py::class_<_SQClosure_, std::shared_ptr<_SQClosure_>>(m, "SQClosure")
    .def("__call__", &_SQClosure_::__call__, py::return_value_policy::move)
    ;

    py::class_<_SQNativeClosure_, std::shared_ptr<_SQNativeClosure_>>(m, "SQNativeClosure")
    .def("__call__", &_SQNativeClosure_::__call__, py::return_value_policy::move)
    ;

    py::class_<_SQArray_, std::shared_ptr<_SQArray_>>(m, "SQArray")
    .def(py::init([](HSQUIRRELVM vm) { return std::make_shared<_SQArray_>(_SQArray_(vm)); }))
    .def("__iter__", &_SQArray_::__iter__, py::return_value_policy::reference_internal)
    .def("__getitem__", &_SQArray_::__getitem__, py::arg("idx"), py::return_value_policy::move)
    .def("__setitem__", &_SQArray_::__setitem__, py::arg("idx"), py::arg("val"), py::keep_alive<1, 3>(), py::return_value_policy::reference)
    .def("append", &_SQArray_::append, py::arg("val"), py::keep_alive<1, 2>())
    .def("pop", &_SQArray_::pop, py::return_value_policy::reference_internal)
    .def("__len__", &_SQArray_::__len__)
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
