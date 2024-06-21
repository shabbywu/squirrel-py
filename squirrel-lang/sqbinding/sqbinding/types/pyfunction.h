#ifndef _SQBINDING_FUNCTION_H_
#define _SQBINDING_FUNCTION_H_

#include "definition.h"
#include "sqfunction.h"
#include "sqstr.h"

namespace py = pybind11;


// PythonNativeCall: wrapper for python function, will not pass squirrel env to python object.
// this function will be used to call python func in SQVM and return result to SQVM
SQInteger PythonNativeCall(HSQUIRRELVM vm);


// PythonNativeRawCall: wrapper for python function, will pass squirrel env to python object.
// this function will be used to call python func in SQVM and return result to SQVM
SQInteger PythonNativeRawCall(HSQUIRRELVM vm);


class SQPythonFunction {
public:
    HSQUIRRELVM vm;
    py::function _val;
    // delegate table
    std::shared_ptr<sqbinding::python::Table> _delegate;
    std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
    std::map<std::string, std::shared_ptr<sqbinding::python::NativeClosure>> nativeclosure_handlers;

    SQPythonFunction(py::function func, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = func;

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
        cppfunction_handlers["_rawcall"] = std::make_shared<py::cpp_function>([this](PyValue env, py::args args) -> PyValue {
            return this->_val.attr("__call__")(*args).cast<PyValue>();
        });

        cppfunction_handlers["_typeof"] = std::make_shared<py::cpp_function>([this]() -> std::string {
            py::type type_ = py::type::of(this->_val);
            return std::string(type_.attr("__module__").cast<std::string>() + "." + type_.attr("__name__").cast<std::string>());
        });

        for(auto& [ k, func ]: cppfunction_handlers) {
            auto withenv = k == "_rawcall";
            nativeclosure_handlers[k] = std::make_shared<sqbinding::python::NativeClosure>(sqbinding::python::NativeClosure{func, vm, withenv? &PythonNativeRawCall: &PythonNativeCall});
        }

        try
        {
            auto _call = nativeclosure_handlers["_call"];
            auto funcName = _val.attr("__name__").cast<py::str>();
            _call->pNativeClosure()->_name = pyvalue_tosqobject(funcName, vm);
        }
        catch(const std::exception& e)
        {

        }

        _delegate = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(vm));
        for(auto pair: nativeclosure_handlers) {
            _delegate->bindFunc(pair.first, pair.second);
        }
    }

    ~SQPythonFunction() {
        release();
    }

    void release() {
        _delegate = NULL;
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release SQPythonFunction" << std::endl;
        #endif
    }

    static SQUserData* Create(py::function func, HSQUIRRELVM vm) {
        // new userdata to store pythonobject
        SQUserPointer ptr = sq_newuserdata(vm, sizeof(SQPythonFunction));
        SQPythonFunction* pycontainer = new(ptr) SQPythonFunction(func, vm);

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->PopGet());
        ud->SetDelegate(pycontainer->_delegate->pTable());
        ud->_hook = release_SQPythonFunction;
        ud->_typetag = (void*)PythonTypeTags::TYPE_FUNCTION;
        return ud;
    }

    static SQInteger release_SQPythonFunction(SQUserPointer ptr, SQInteger size) {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release callback release_SQPythonFunction" << std::endl;
        #endif
        SQPythonFunction* ref = (SQPythonFunction*)(ptr);
        ref->release();
        return 1;
    }
};
#endif
