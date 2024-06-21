#include "definition.h"
#include "container.h"
#include "sqiterator.h"
#include "sqobject.h"


std::string sqobject_to_string(SQObjectPtr& self) {
    switch (self._type) {
        case tagSQObjectType::OT_NULL:
            return std::string("OT_NULL");
        case tagSQObjectType::OT_INTEGER:
            return string_format("OT_INTEGER: {%d}", _integer(self));
        case tagSQObjectType::OT_FLOAT:
            return string_format("OT_FLOAT: {%f}", _float(self));
        case tagSQObjectType::OT_BOOL:
            return string_format("OT_BOOL: {%s}", _integer(self)?"true":"false");
        case tagSQObjectType::OT_STRING:
            return string_format("OT_STRING: {%s}", _stringval(self));
        case tagSQObjectType::OT_TABLE:
            return string_format("OT_TABLE: {%p}[{%p}]", _table(self), _table(self)->_delegate);
        case tagSQObjectType::OT_ARRAY:
            return string_format("OT_ARRAY: {%p}", _array(self));
        case tagSQObjectType::OT_USERDATA:
            return string_format("OT_USERDATA: {%p}[{%p}]", _userdataval(self),_userdata(self)->_delegate);
        case tagSQObjectType::OT_CLOSURE:
            return string_format("OT_CLOSURE: [{%p}]", _closure(self));
        case tagSQObjectType::OT_NATIVECLOSURE:
            return string_format("OT_NATIVECLOSURE: [{%p}]", _nativeclosure(self));
        case tagSQObjectType::OT_GENERATOR:
            return string_format("OT_GENERATOR: [{%p}]", _generator(self));
        case tagSQObjectType::OT_USERPOINTER:
            return string_format("OT_USERPOINTER: [{%p}]", _userpointer(self));
        case tagSQObjectType::OT_THREAD:
            return string_format("OT_THREAD: [{%p}]", _thread(self));
        case tagSQObjectType::OT_FUNCPROTO:
            return string_format("OT_FUNCPROTO: [{%p}]", _funcproto(self));
        case tagSQObjectType::OT_CLASS:
            return string_format("OT_CLASS: [{%p}]", _class(self));
        case tagSQObjectType::OT_INSTANCE:
            return string_format("OT_INSTANCE: [{%p}]", _instance(self));
        case tagSQObjectType::OT_WEAKREF:
            return string_format("OT_WEAKREF: [{%p}]", _weakref(self));
        case tagSQObjectType::OT_OUTER:
            return string_format("OT_OUTER: [{%p}]", _outer(self));
        default:
            return string_format("TYPE_UNKNOWN: [{%p}]", &self);
    }
}


#define __try_cast_pyuserdata_to_python(object, typetag, cpp_type) \
if (_userdata(object)->_typetag == (void*)typetag) {\
    return ((cpp_type*)(sq_aligning(_userdata(object) + 1)))->_val;\
}


PyValue sqobject_topython(SQObjectPtr& object, HSQUIRRELVM vm) {
    switch (sq_type(object))
    {
    case tagSQObjectType::OT_NULL:
        return PyValue(py::none());
    case tagSQObjectType::OT_INTEGER:
        return PyValue(py::int_(_integer(object)));
    case tagSQObjectType::OT_FLOAT:
        return PyValue(py::float_(_float(object)));
    case tagSQObjectType::OT_BOOL:
        return PyValue(py::bool_(_integer(object)));
    case tagSQObjectType::OT_STRING:
        #ifdef USE__SQString__
        return PyValue(std::make_shared<sqbinding::python::String>(sqbinding::python::String(_string(object), vm)));
        #else
        return std::string(_stringval(object));
        #endif
    case tagSQObjectType::OT_ARRAY:
        return std::move(std::shared_ptr<sqbinding::python::Array>(new sqbinding::python::Array{_array(object), vm}));
    case tagSQObjectType::OT_TABLE:
        return std::move(std::shared_ptr<sqbinding::python::Table>(new sqbinding::python::Table{_table(object), vm}));
    case tagSQObjectType::OT_CLASS:
        return std::move(std::shared_ptr<sqbinding::python::Class>(new sqbinding::python::Class{_class(object), vm}));
    case tagSQObjectType::OT_INSTANCE:
        return std::move(std::shared_ptr<sqbinding::python::Instance>(new sqbinding::python::Instance{_instance(object), vm}));
    case tagSQObjectType::OT_CLOSURE:
        return std::move(std::shared_ptr<sqbinding::python::Closure>(new sqbinding::python::Closure{_closure(object), vm}));
    case tagSQObjectType::OT_NATIVECLOSURE:
        return std::move(std::shared_ptr<sqbinding::python::NativeClosure>(new sqbinding::python::NativeClosure{_nativeclosure(object), vm}));
    case tagSQObjectType::OT_USERDATA:
        {
            __try_cast_pyuserdata_to_python(object, PythonTypeTags::TYPE_LIST, SQPythonList)
            __try_cast_pyuserdata_to_python(object, PythonTypeTags::TYPE_DICT, SQPythonDict)
            __try_cast_pyuserdata_to_python(object, PythonTypeTags::TYPE_FUNCTION, SQPythonFunction)
            __try_cast_pyuserdata_to_python(object, PythonTypeTags::TYPE_OBJECT, SQPythonObject)
        }
    default:
        std::cout << "cast unknown obj to python: " << sqobject_to_string(object) << std::endl;
        return py::none();
    }
}
