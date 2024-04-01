#ifndef _SQBINDING_DICT_H_
#define _SQBINDING_DICT_H_

#include "definition.h"
#include "sqiterator.h"


#ifdef USE__SQString__
#include "sqstr.h"
#define TYPE_KEY _SQString_
#else
#define TYPE_KEY std::string
#endif


namespace py = pybind11;



class SQPythonDict {
public:
    static int typetag;
    HSQUIRRELVM vm;
    py::dict _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;

    py::cpp_function _get;
    py::cpp_function _set;
    py::cpp_function _newslot;
    py::cpp_function _delslot;

    py::cpp_function clear;
    py::cpp_function pop;
    py::cpp_function len;

    SQPythonDict(py::dict dict, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = dict;

        _get = py::cpp_function([this](TYPE_KEY key) -> PyValue {
            return this->_val[py::str(key)].cast<PyValue>();
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

        pop = py::cpp_function([this](TYPE_KEY key, PyValue value) -> PyValue {
            return this->_val.attr("pop")(key, value);
        });
        len = py::cpp_function([this]() -> PyValue {
            return py::int_(this->_val.size());
        });
        clear = py::cpp_function([this]() {
            this->_val.clear();
        });

        _delegate = std::make_shared<_SQTable_>(_SQTable_(vm));
        _delegate->bindFunc("_get", _get);
        _delegate->bindFunc("_set", _set);
        _delegate->bindFunc("_newslot", _newslot);
        _delegate->bindFunc("_delslot", _delslot);

        _delegate->bindFunc("clear", clear);
        _delegate->bindFunc("pop", pop);
        _delegate->bindFunc("len", len);
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
        SQPythonDict* pycontainer = new SQPythonDict(dict, vm);

        SQUserPointer ptr = sq_newuserdata(vm, sizeof(SQPythonDict));
        std::memcpy(ptr, pycontainer, sizeof(SQPythonDict));

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->Top());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = release_SQPythonDict;
        // ud->_typetag = &SQPythonDict::typetag;
        return ud;
    }

    static SQInteger release_SQPythonDict(SQUserPointer ptr, SQInteger size) {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release callback release_SQPythonDict" << std::endl;
        #endif
        SQPythonDict* ref = (SQPythonDict*)(ptr);
        ref->release();
        return 1;
    }
};
#endif
