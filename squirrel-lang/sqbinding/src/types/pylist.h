#ifndef _SQBINDING_LIST_H_
#define _SQBINDING_LIST_H_

#include "definition.h"
#include "sqiterator.h"

namespace py = pybind11;


class SQPythonList {
public:
    HSQUIRRELVM vm;
    py::list _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;
    std::map<std::string, py::cpp_function> cppfunction_handlers;
    std::map<std::string, std::shared_ptr<_SQNativeClosure_>> nativeclosure_handlers;

    SQPythonList(py::list list, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = list;

        cppfunction_handlers["_get"] = py::cpp_function([this](py::int_ key) -> PyValue {
            return this->_val[key].cast<PyValue>();
        });
        cppfunction_handlers["_set"] = py::cpp_function([this](py::int_ key, PyValue value){
            this->_val.attr("__setitem__")(key, value);
        });
        cppfunction_handlers["_newslot"] = py::cpp_function([this](py::int_ key, PyValue value){
            this->_val.attr("__setitem__")(key, value);
        });
        cppfunction_handlers["_delslot"] = py::cpp_function([this](py::int_ key) {
            this->_val.attr("__delitem__")(key);
        });

        cppfunction_handlers["append"] = py::cpp_function([this](PyValue value){
            this->_val.attr("append")(value);
        });
        cppfunction_handlers["pop"] = py::cpp_function([this](PyValue value) -> PyValue {
            return this->_val.attr("pop")(value);
        });
        cppfunction_handlers["len"] = py::cpp_function([this]() -> PyValue {
            return this->_val.attr("__len__")();
        });

        for(const auto& [ k, v ]: cppfunction_handlers) {
            nativeclosure_handlers[k] = std::make_shared<_SQNativeClosure_>(_SQNativeClosure_{v, vm});
        }

        _delegate = std::make_shared<_SQTable_>(_SQTable_(vm));
        for(auto pair: nativeclosure_handlers) {
            _delegate->bindFunc(pair.first, pair.second);
        }
    }

    ~SQPythonList() {
        release();
    }

    void release() {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release SQPythonList" << std::endl;
        #endif
        _delegate = NULL;
    }

    static SQUserData* Create(py::list list, HSQUIRRELVM vm) {
        // new userdata to store pythonlist
        SQPythonList* pycontainer = new SQPythonList(list, vm);

        SQUserPointer ptr = sq_newuserdata(vm, sizeof(SQPythonList));
        std::memcpy(ptr, pycontainer, sizeof(SQPythonList));

        // get userdata in stack top
        SQUserData* ud = _userdata(vm->PopGet());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = release_SQPythonList;
        ud->_typetag = &PythonTypeTag::list;
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
