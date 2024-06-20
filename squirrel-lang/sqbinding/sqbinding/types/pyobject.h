#ifndef _SQBINDING_PYOBJECT_H_
#define _SQBINDING_PYOBJECT_H_

#include "definition.h"
#include "sqiterator.h"
#include "sqstr.h"
#include "pyfunction.h"


namespace py = pybind11;


class SQPythonObject {
public:

    HSQUIRRELVM vm;
    py::object _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;
    std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
    std::map<std::string, std::shared_ptr<sqbinding::python::NativeClosure>> nativeclosure_handlers;

    SQPythonObject(py::object object, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = object;

        cppfunction_handlers["_get"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key) -> PyValue {
            return this->_val.attr("__getattribute__")(key).cast<PyValue>();
        });

        cppfunction_handlers["_set"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value) -> SQBool {
            this->_val.attr("__setattr__")(key, value);
            return 0;
        });
        cppfunction_handlers["_newslot"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key, PyValue value){
            this->_val.attr("__setattr__")(key, value);
        });
        cppfunction_handlers["_delslot"] = std::make_shared<py::cpp_function>([this](sqbinding::detail::string key) {
            this->_val.attr("__delattr__")(key);
        });

        cppfunction_handlers["_call"] = std::make_shared<py::cpp_function>([this](PyValue env, py::args args) -> PyValue {
            return this->_val.attr("__call__")(*args).cast<PyValue>();
        });
        cppfunction_handlers["_typeof"] = std::make_shared<py::cpp_function>([this]() -> std::string {
            py::type type_ = py::type::of(this->_val);
            return std::string(type_.attr("__module__").cast<std::string>() + "." + type_.attr("__name__").cast<std::string>());
        });

        for(auto& [ k, v ]: cppfunction_handlers) {
            nativeclosure_handlers[k] = std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure{v, vm, &PythonNativeCall});
        }

        _delegate = std::make_shared<_SQTable_>(_SQTable_(vm));
        for(auto pair: nativeclosure_handlers) {
            _delegate->bindFunc(pair.first, pair.second);
        }
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
        SQUserData* ud = _userdata(vm->PopGet());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = release_SQPythonObject;
        ud->_typetag = (void*)PythonTypeTags::TYPE_OBJECT;
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
