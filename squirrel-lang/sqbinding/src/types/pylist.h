#ifndef _SQBINDING_LIST_H_
#define _SQBINDING_LIST_H_

#include "definition.h"
#include "sqiterator.h"


#ifdef USE__SQString__
#include "sqstr.h"
#define TYPE_KEY _SQString_
#else
#define TYPE_KEY std::string
#endif


namespace py = pybind11;



class SQPythonList {
public:
    static int typetag;
    HSQUIRRELVM vm;
    py::list _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;

    py::cpp_function _get;
    py::cpp_function _set;
    py::cpp_function _newslot;
    py::cpp_function _delslot;

    SQPythonList(py::list list, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = list;

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

    ~SQPythonList() {
        release();
    }

    void release() {
        _delegate = NULL;
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release SQPythonList" << std::endl;
        #endif
    }

    static SQUserData* Create(py::list list, HSQUIRRELVM vm) {
        // new userdata to store pythonlist
        SQPythonList* pycontainer = new SQPythonList(list, vm);

        SQUserPointer ptr = sq_newuserdata(vm, sizeof(SQPythonList));
        std::memcpy(ptr, pycontainer, sizeof(SQPythonList));

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->Top());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = release_SQPythonList;
        // ud->_typetag = &SQPythonList::typetag;
        return ud;
    }

    static SQInteger release_SQPythonList(SQUserPointer ptr, SQInteger size) {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release callback release_SQPythonList" << std::endl;
        #endif
        SQPythonList* ref = (SQPythonList*)(ptr);
        ref->release();
        return 1;
    }
};
#endif
