#ifndef _SQBINDING_PYOBJECT_H_
#define _SQBINDING_PYOBJECT_H_

#include "definition.h"
#include "sqiterator.h"

#ifdef USE__SQString__
#include "sqstr.h"
#define TYPE_KEY _SQString_
#else
#define TYPE_KEY std::string
#endif


namespace py = pybind11;



class SQPythonObject {
public:
    static int typetag;
    HSQUIRRELVM vm;
    py::object _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;

    py::cpp_function _get;
    py::cpp_function _set;
    py::cpp_function _newslot;
    py::cpp_function _delslot;

    py::cpp_function _call;
    py::cpp_function _typeof;

    SQPythonObject(py::object object, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = object;

        _get = py::cpp_function([this](TYPE_KEY key) -> PyValue {
            return this->_val[py::str(key)].cast<PyValue>();
        });
        _set = py::cpp_function([this](TYPE_KEY key, PyValue value) -> SQBool {
            this->_val.attr("__setattr__")(key, value);
            return 0;
        });
        _newslot = py::cpp_function([this](TYPE_KEY key, PyValue value){
            this->_val.attr("__setattr__")(key, value);
        });
        _delslot = py::cpp_function([this](TYPE_KEY key) {
            this->_val.attr("__delattr__")(key);
        });

        _call = py::cpp_function([this](py::args args) -> py::object {
            return this->_val.attr("__call__")(*args);
        });

        _typeof = py::cpp_function([this]() -> std::string {
            py::type type_ = py::type::of(this->_val);
            return std::string(type_.attr("__module__").cast<std::string>() + "." + type_.attr("__name__").cast<std::string>());
        });

        _delegate = std::make_shared<_SQTable_>(_SQTable_(vm));
        _delegate->bindFunc("_get", _get);
        _delegate->bindFunc("_set", _set);
        _delegate->bindFunc("_newslot", _newslot);
        _delegate->bindFunc("_delslot", _delslot);
        _delegate->bindFunc("_call", _call);
        _delegate->bindFunc("_typeof", _typeof);

    }

    ~SQPythonObject() {
        release();
    }

    void release() {
        _delegate = NULL;
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release SQPythonObject" << std::endl;
        #endif
    }

    static SQUserData* Create(py::object object, HSQUIRRELVM vm) {
        // new userdata to store pythonobject
        SQPythonObject* pycontainer = new SQPythonObject(object, vm);

        SQUserPointer ptr = sq_newuserdata(vm, sizeof(SQPythonObject));
        std::memcpy(ptr, pycontainer, sizeof(SQPythonObject));

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->Top());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = release_SQPythonObject;
        // ud->_typetag = &SQPythonObject::typetag;
        return ud;
    }

    static SQInteger release_SQPythonObject(SQUserPointer ptr, SQInteger size) {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release callback release_SQPythonObject" << std::endl;
        #endif
        SQPythonObject* ref = (SQPythonObject*)(ptr);
        ref->release();
        return 1;
    }
};
#endif
