#ifndef _SQBINDING_DICT_H_
#define _SQBINDING_DICT_H_

#include "definition.h"
#include "sqiterator.h"
#include "object.h"


namespace py = pybind11;


class SQPythonDict {
public:
    py::dict _val;
    HSQUIRRELVM vm;
    // delegate table
    SQObjectPtr obj;

    py::cpp_function _get;
    py::cpp_function _set;
    py::cpp_function _newslot;

    SQPythonDict(py::dict dict, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = dict;

        sq_newtable(vm);
        sq_getstackobj(vm,-1,&obj);
        sq_addref(vm, &obj);
        sq_pop(vm,1);

        _get = py::cpp_function([this](std::string key) -> py::object {
            return this->_val.attr("__getitem__")(key);
        });
        _set = py::cpp_function([this](std::string key, PyValue value){
            auto tmp = pyvalue_tosqobject(value, this->vm);
            this->_val.attr("__setitem__")(key, value);
        });
        _newslot = py::cpp_function([this](std::string key, PyValue value){
            this->_val.attr("__setitem__")(key, value);
        });

        auto _delegate = _SQTable_(_table(obj), vm);
        _delegate.bindFunc("_get", _get);
        _delegate.bindFunc("_set", _set);
        _delegate.bindFunc("_newslot", _newslot);
    }

    ~SQPythonDict() {
        Release();
    }

    void Release() {
        // std::cout << "release delegate table " << _table(obj)->_uiRef << std::endl;
        _table(obj)->Clear();
        sq_release(vm, &obj);
        sq_resetobject(&obj);
        // std::cout << "release delegate table done" << std::endl;
    }

    static SQUserData* Create(py::dict dict, HSQUIRRELVM vm) {
        SQPythonDict* delegate = new SQPythonDict(dict, vm);
        // TODO: 根据 dict 生成唯一 key
        auto key = vm->PrintObjVal(SQInteger(reinterpret_cast<intptr_t>((void*)dict.ptr())));

        sq_pushregistrytable(vm);
        sq_pushstring(vm, key->_val, -1);

        SQUserPointer ptr = sq_newuserdata(vm, sizeof(delegate));
        std::memcpy(ptr, &delegate, sizeof(delegate));

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->Top());
        sq_newslot(vm, -3, SQFalse);
        // pop table
        sq_pop(vm, 1);

        ud->SetDelegate(_table(delegate->obj));
        ud->_hook = releasePydictRef;
        return ud;
    }

    static SQInteger releasePydictRef(SQUserPointer ptr, SQInteger size) {
        // std::cout << "calling release_pydict_ref callback" << std::endl;
        SQPythonDict** ref = (SQPythonDict**)(ptr);
        delete *ref;
        return 1;
    }
};
#endif
