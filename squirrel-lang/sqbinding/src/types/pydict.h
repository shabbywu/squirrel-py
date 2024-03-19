#ifndef _SQBINDING_DICT_H_
#define _SQBINDING_DICT_H_

#include "definition.h"
#include "sqiterator.h"
#include "sqstr.h"


#ifdef USE__SQString__
#define TYPE_KEY _SQString_
#else
#define TYPE_KEY std::string
#endif


namespace py = pybind11;



class SQPythonDict {
public:
    HSQUIRRELVM vm;
    py::dict _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;

    py::cpp_function _get;
    py::cpp_function _set;
    py::cpp_function _newslot;
    py::cpp_function _delslot;

    SQPythonDict(py::dict dict, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = dict;

        _get = py::cpp_function([this](TYPE_KEY key) -> py::object {
            return this->_val.attr("__getitem__")(key);
        });
        _set = py::cpp_function([this](TYPE_KEY key, PyValue value){
            this->_val.attr("__setitem__")(key, value);
        });
        _newslot = py::cpp_function([this](TYPE_KEY key, PyValue value){
            this->_val.attr("__setitem__")(key, value);
        });
        _delslot = py::cpp_function([this](TYPE_KEY key) {
            this->_val.attr("__delitem__")(key);
        });

        _delegate = std::make_shared<_SQTable_>(_SQTable_(vm));
        _delegate->bindFunc("_get", _get);
        _delegate->bindFunc("_set", _set);
        _delegate->bindFunc("_newslot", _newslot);
        _delegate->bindFunc("_delslot", _delslot);
    }

    ~SQPythonDict() {
        release();
    }

    void release() {
        _delegate = NULL;
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release SQPythonDict" << std::endl;
        #endif
    }

    static SQUserData* Create(py::dict dict, HSQUIRRELVM vm) {
        // new userdata to store pythondict
        std::cout << "before SQPythonDict" << std::endl;
        SQPythonDict* pycontainer = new SQPythonDict(dict, vm);
        std::cout << "after SQPythonDict" << std::endl;

        SQUserPointer ptr = sq_newuserdata(vm, sizeof(SQPythonDict));
        std::memcpy(ptr, pycontainer, sizeof(SQPythonDict));

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->Top());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = releasePydictRef;
        return ud;
    }

    static SQInteger releasePydictRef(SQUserPointer ptr, SQInteger size) {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release callback releasePydictRef" << std::endl;
        #endif
        SQPythonDict* ref = (SQPythonDict*)(ptr);
        ref->release();
        return 1;
    }
};
#endif
