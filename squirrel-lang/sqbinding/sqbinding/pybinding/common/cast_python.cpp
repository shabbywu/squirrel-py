#include "cast.h"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/pybinding/types/definition.h"
#include "sqbinding/pybinding/types/container.h"
#include "sqbinding/pybinding/types/sqobject.hpp"

#ifdef TRACE_OBJECT_CAST
#define __try_cast_cppwrapper_tosqobject(object, type, field) \
if (std::holds_alternative<std::shared_ptr<type>>(object)) {\
    auto _val = std::get<std::shared_ptr<type>>(object);\
    std::cout << "[TRACING] cast PyValue to " << typeid(decltype(_val->field)).name() << std::endl;\
    return _val->field;\
}
#else
#define __try_cast_cppwrapper_tosqobject(object, type, field) \
if (std::holds_alternative<std::shared_ptr<type>>(object)) {\
    auto _val = std::get<std::shared_ptr<type>>(object);\
    return _val->field;\
}
#endif

SQObjectPtr sqbinding::python::pyvalue_tosqobject(PyValue value, detail::VM vm) {

    // 尝试解包装
    if (std::holds_alternative<py::object>(value)) {
        value = pyobject_topyvalue(std::get<py::object>(value));
    }

    if (std::holds_alternative<py::none>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to OT_NULL" << std::endl;
        #endif
        return SQObjectPtr();
    } else if (std::holds_alternative<py::int_>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQInteger).name() << std::endl;
        #endif
        return SQObjectPtr((SQInteger)std::get<py::int_>(value));
    } else if (std::holds_alternative<py::float_>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQFloat).name() << std::endl;
        #endif
        return SQObjectPtr((SQFloat)std::get<py::float_>(value));
    } else if (std::holds_alternative<py::bool_>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(bool).name() << std::endl;
        #endif
        return SQObjectPtr(bool(std::get<py::bool_>(value)));
    } else if (std::holds_alternative<std::string>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQString).name() << std::endl;
        #endif
        std::string s = std::get<std::string>(value);
        return SQObjectPtr(SQString::Create(_ss(*vm), s.c_str(), s.size()));
    } else if (std::holds_alternative<py::list>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQPythonList).name() << std::endl;
        #endif
        return SQObjectPtr(SQPythonList::Create(std::get<py::list>(value), vm));
    } else if (std::holds_alternative<py::dict>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQPythonDict).name() << std::endl;
        #endif
        return SQObjectPtr(SQPythonDict::Create(std::get<py::dict>(value), vm));
    } else if (std::holds_alternative<py::function>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQPythonFunction).name() << std::endl;
        #endif
        return SQObjectPtr(SQPythonFunction::Create(std::get<py::function>(value), vm));
    } else if (std::holds_alternative<py::type>(value)) {
        // TODO: 支持复杂的类型代理
        // return SQObjectPtr();
    }
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::String, pString())
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::Array, pArray())
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::Table, pTable())
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::Class, pClass())
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::Instance, pInstance())
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::Closure, pClosure())
    __try_cast_cppwrapper_tosqobject(value, sqbinding::python::NativeClosure, pNativeClosure())

    if (std::holds_alternative<py::object>(value)) {
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast PyValue to " << typeid(SQPythonObject).name() << std::endl;
        #endif
        return SQObjectPtr(SQPythonObject::Create(std::get<py::object>(value), vm));
    }

    std::cout << "varient index=" << value.index() << std::endl;
    throw py::value_error("can't cast this value to SQObjectPtr, index=" + value.index());
};


#define __try_cast_pyobject_topyvalue(v, object, type) \
if (py::isinstance<type>(object)) {\
    return py::cast<type>(object); \
}


#define __try_cast_pyobject_to_ptr(v, object, type) \
if (py::isinstance<type>(object)) {\
    return std::make_shared<type>(py::cast<type>(object)); \
}


PyValue sqbinding::python::pyobject_topyvalue(py::object object) {
    __try_cast_pyobject_topyvalue(v, object, py::none)
    if (py::isinstance<py::int_>(object)) {
        return py::cast<py::int_>(object);
    }
    if (py::isinstance<py::float_>(object)) {
        return py::cast<py::float_>(object);
    }
    if (py::isinstance<py::str>(object)) {
        return (std::string)py::cast<py::str>(object);
    }
    __try_cast_pyobject_topyvalue(v, object, py::float_)
    __try_cast_pyobject_topyvalue(v, object, py::bool_)
    __try_cast_pyobject_topyvalue(v, object, std::string)
    __try_cast_pyobject_topyvalue(v, object, py::list)
    __try_cast_pyobject_topyvalue(v, object, py::dict)
    __try_cast_pyobject_topyvalue(v, object, py::function)
    __try_cast_pyobject_topyvalue(v, object, py::type)

    #ifdef USE__SQString__
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::String>)
    #else
    __try_cast_pyobject_topyvalue(v, object, std::string)
    #endif
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::Array>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::Array)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::ArrayIterator>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::ArrayIterator)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::Table>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::Table)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::TableIterator>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::TableIterator)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::Class>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::Class)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::Instance>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::Instance)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::Closure>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::Closure)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<sqbinding::python::NativeClosure>)
    __try_cast_pyobject_to_ptr(v, object, sqbinding::python::NativeClosure)

    return object;
}


#define __try_cast_pyuserdata_to_python(object, typetag, cpp_type) \
if (_userdata(object)->_typetag == (void*)typetag) {\
    return ((cpp_type*)(sq_aligning(_userdata(object) + 1)))->_val;\
}

#ifdef TRACE_OBJECT_CAST
#define __try_cast_userdata(object, typetag, cpp_type) \
if (_userdata(object)->_typetag == (void*)typetag) {\
    using Holder = detail::StackObjectHolder<cpp_type>;\
    Holder* holder = ((Holder*)(sq_aligning(_userdata(object) + 1)));\
    std::cout << "[TRACING] cast SQObjectPtr to " << typeid(decltype(holder->GetInstance()._val)).name() << std::endl;\
    return holder->GetInstance()._val;\
}
#else
#define __try_cast_userdata(object, typetag, cpp_type) \
if (_userdata(object)->_typetag == (void*)typetag) {\
    using Holder = detail::StackObjectHolder<cpp_type>;\
    Holder* holder = ((Holder*)(sq_aligning(_userdata(object) + 1)));\
    return holder->GetInstance()._val;\
}
#endif


PyValue sqbinding::python::sqobject_topython(SQObjectPtr& object, detail::VM vm) {
    switch (sq_type(object))
    {
    case tagSQObjectType::OT_NULL:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(py::none).name() << std::endl;
        #endif
        return PyValue(py::none());
    case tagSQObjectType::OT_INTEGER:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(py::int_).name() << std::endl;
        #endif
        return PyValue(py::int_(_integer(object)));
    case tagSQObjectType::OT_FLOAT:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(py::float_).name() << std::endl;
        #endif
        return PyValue(py::float_(_float(object)));
    case tagSQObjectType::OT_BOOL:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(py::bool_).name() << std::endl;
        #endif
        return PyValue(py::bool_(_integer(object)));
    case tagSQObjectType::OT_STRING:
        #ifdef USE__SQString__
        return PyValue(std::make_shared<sqbinding::python::String>(sqbinding::python::String(_string(object), vm)));
        #else
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to std::string"<< std::endl;
        #endif
        return std::string(_stringval(object));
        #endif
    case tagSQObjectType::OT_ARRAY:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(sqbinding::python::Array).name() << std::endl;
        #endif
        return std::make_shared<sqbinding::python::Array>(_array(object), vm);
    case tagSQObjectType::OT_TABLE:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(sqbinding::python::Table).name() << std::endl;
        #endif
        return std::make_shared<sqbinding::python::Table>(_table(object), vm);
    case tagSQObjectType::OT_CLASS:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(sqbinding::python::Class).name() << std::endl;
        #endif
        return std::make_shared<sqbinding::python::Class>(_class(object), vm);
    case tagSQObjectType::OT_INSTANCE:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(sqbinding::python::Instance).name() << std::endl;
        #endif
        return std::make_shared<sqbinding::python::Instance>(_instance(object), vm);
    case tagSQObjectType::OT_CLOSURE:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(sqbinding::python::Closure).name() << std::endl;
        #endif
        return std::make_shared<sqbinding::python::Closure>(_closure(object), vm);
    case tagSQObjectType::OT_NATIVECLOSURE:
        #ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(sqbinding::python::NativeClosure).name() << std::endl;
        #endif
        return std::make_shared<sqbinding::python::NativeClosure>(_nativeclosure(object), vm);
    case tagSQObjectType::OT_USERDATA:
        {
            __try_cast_userdata(object, PythonTypeTags::TYPE_LIST, SQPythonList)
            __try_cast_userdata(object, PythonTypeTags::TYPE_DICT, SQPythonDict)
            __try_cast_userdata(object, PythonTypeTags::TYPE_FUNCTION, SQPythonFunction)
            __try_cast_userdata(object, PythonTypeTags::TYPE_OBJECT, SQPythonObject)
        }
    default:
        std::cout << "cast unknown obj to python: " << detail::sqobject_to_string(object) << std::endl;
        return py::none();
    }
}
