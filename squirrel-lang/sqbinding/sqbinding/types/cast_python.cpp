#include "definition.h"
#include "container.h"
#include "sqiterator.h"
#include "object.h"


#define __try_cast_pyvalue_tosqobject(object, type) \
if (std::holds_alternative<std::shared_ptr<type>>(object)) {\
    auto _val = std::get<std::shared_ptr<type>>(object);\
    return _val->obj;\
}


#define __try_cast_cppwrapper_tosqobject(object, type, field) \
if (std::holds_alternative<std::shared_ptr<type>>(object)) {\
    auto _val = std::get<std::shared_ptr<type>>(object);\
    return _val->field;\
}


SQObjectPtr pyvalue_tosqobject(PyValue value, HSQUIRRELVM vm) {
    // 尝试解包装
    if (std::holds_alternative<py::object>(value)) {
        value = pyobject_topyvalue(std::get<py::object>(value));
    }

    if (std::holds_alternative<py::none>(value)) {
        return SQObjectPtr();
    } else if (std::holds_alternative<py::int_>(value)) {
        return SQObjectPtr((SQInteger)std::get<py::int_>(value));
    } else if (std::holds_alternative<py::float_>(value)) {
        return SQObjectPtr((SQFloat)std::get<py::float_>(value));
    } else if (std::holds_alternative<py::bool_>(value)) {
        return SQObjectPtr(bool(std::get<py::bool_>(value)));
    } else if (std::holds_alternative<std::string>(value)) {
        std::string s = std::get<std::string>(value);
        return SQObjectPtr(SQString::Create(_ss(vm), s.c_str(), s.size()));
    } else if (std::holds_alternative<py::list>(value)) {
        return SQObjectPtr(SQPythonList::Create(std::get<py::list>(value), vm));
    } else if (std::holds_alternative<py::dict>(value)) {
        return SQObjectPtr(SQPythonDict::Create(std::get<py::dict>(value), vm));
    } else if (std::holds_alternative<py::function>(value)) {
        return SQObjectPtr(SQPythonFunction::Create(std::get<py::function>(value), vm));
    } else if (std::holds_alternative<py::type>(value)) {
        // TODO: 支持复杂的类型代理
        // return SQObjectPtr();
    }
    __try_cast_pyvalue_tosqobject(value, _SQString_)
    __try_cast_cppwrapper_tosqobject(value, _SQArray_, pArray)
    __try_cast_cppwrapper_tosqobject(value, _SQTable_, pTable)
    __try_cast_cppwrapper_tosqobject(value, _SQClass_, pClass)
    __try_cast_cppwrapper_tosqobject(value, _SQInstance_, pInstance)
    __try_cast_cppwrapper_tosqobject(value, _SQClosure_, pClosure)
    __try_cast_cppwrapper_tosqobject(value, _SQNativeClosure_, pNativeClosure)

    if (std::holds_alternative<py::object>(value)) {
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


PyValue pyobject_topyvalue(py::object object) {
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
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQString_>)
    #else
    __try_cast_pyobject_topyvalue(v, object, std::string)
    #endif
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQArray_>)
    __try_cast_pyobject_to_ptr(v, object, _SQArray_)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<ArrayIterator>)
    __try_cast_pyobject_to_ptr(v, object, ArrayIterator)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQTable_>)
    __try_cast_pyobject_to_ptr(v, object, _SQTable_)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<TableIterator>)
    __try_cast_pyobject_to_ptr(v, object, TableIterator)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQClass_>)
    __try_cast_pyobject_to_ptr(v, object, _SQClass_)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQInstance_>)
    __try_cast_pyobject_to_ptr(v, object, _SQInstance_)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQClosure_>)
    __try_cast_pyobject_to_ptr(v, object, _SQClosure_)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQNativeClosure_>)
    __try_cast_pyobject_to_ptr(v, object, _SQNativeClosure_)

    return object;
}
