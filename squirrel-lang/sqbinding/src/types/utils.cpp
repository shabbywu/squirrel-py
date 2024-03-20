#include "definition.h"
#include "container.h"
#include "sqiterator.h"
#include "object.h"
#include "sqfunction.h"


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
        return PyValue(std::make_shared<_SQString_>(_SQString_(_string(object), vm)));
        #else
        return std::string(_stringval(object));
        #endif
    case tagSQObjectType::OT_ARRAY:
        return PyValue(std::make_shared<_SQArray_>(_SQArray_(_array(object), vm)));
    case tagSQObjectType::OT_TABLE:
        return PyValue(std::make_shared<_SQTable_>(_SQTable_(_table(object), vm)));
    case tagSQObjectType::OT_CLASS:
        return PyValue(std::make_shared<_SQClass_>(_SQClass_(_class(object), vm)));
    case tagSQObjectType::OT_INSTANCE:
        return std::make_shared<_SQInstance_>(_SQInstance_(_instance(object), vm));
    case tagSQObjectType::OT_CLOSURE:
        return PyValue(std::make_shared<_SQClosure_>(_SQClosure_(_closure(object), vm)));
    case tagSQObjectType::OT_NATIVECLOSURE:
        return PyValue(std::make_shared<_SQNativeClosure_>(_SQNativeClosure_(_nativeclosure(object), vm)));
    default:
        std::cout << "cast unknown obj to python: " << sqobject_to_string(object) << std::endl;
        return PyValue(py::none());
    }
}


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
        py::type T_func = py::type::of(std::get<py::function>(value));
        if (T_func.attr("__module__").cast<std::string>() != "builtins") {
            // handle callable object
            return SQObjectPtr(SQPythonObject::Create(std::get<py::function>(value), vm));
        }
        // handle builtins.function && builtins.method
        // TODO: 修改成直接调用 SQNativeClosure::Create (避免依赖 vm)
        py::function func = std::get<py::function>(value);
        // store py::function to userdata
        SQUserPointer ptr = sq_newuserdata(vm, sizeof(func));
        std::memcpy(ptr, &func, sizeof(func));
        sq_newclosure(vm, PythonNativeCall, 1);
        // store and pop the closure in stack top
        SQObjectPtr closure = vm->Top();
        vm->Pop();
        return closure;
    } else if (std::holds_alternative<py::type>(value)) {
        // TODO: 支持复杂的类型代理
        // return SQObjectPtr();
    }
    __try_cast_pyvalue_tosqobject(value, _SQString_)
    __try_cast_cppwrapper_tosqobject(value, _SQArray_, pArray)
    __try_cast_cppwrapper_tosqobject(value, _SQTable_, pTable)
        // __try_cast_cppwrapper_tosqobject(value, _SQClass_)
    __try_cast_cppwrapper_tosqobject(value, _SQInstance_, pInstance)

    __try_cast_pyvalue_tosqobject(value, _SQClosure_)
    __try_cast_pyvalue_tosqobject(value, _SQNativeClosure_)

    if (std::holds_alternative<py::object>(value)) {
        return SQObjectPtr(SQPythonObject::Create(std::get<py::object>(value), vm));
    }

    std::cout << "varient index=" << value.index() << std::endl;
    throw py::value_error("can't cast this value to SQObjectPtr, index=" + value.index());
};


#define __try_cast_pyobject_topyvalue(v, object, type) \
if (py::isinstance<type>(object)) {\
    v = py::cast<type>(object); \
}


PyValue pyobject_topyvalue(py::object object) {
    PyValue v = object;
    __try_cast_pyobject_topyvalue(v, object, py::none)
    if (py::isinstance<py::int_>(object)) {
        v = py::cast<py::int_>(object);
    }
    if (py::isinstance<py::float_>(object)) {
        v = py::cast<py::float_>(object);
    }
    if (py::isinstance<py::str>(object)) {
        v = (std::string)py::cast<py::str>(object);
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
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<ArrayIterator>)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQTable_>)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<TableIterator>)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQClass_>)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQInstance_>)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQClosure_>)
    __try_cast_pyobject_topyvalue(v, object, std::shared_ptr<_SQNativeClosure_>)

    return v;
}
