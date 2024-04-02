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
    HSQUIRRELVM vm;
    py::dict _val;
    // delegate table
    std::shared_ptr<_SQTable_> _delegate;
    std::map<std::string, std::shared_ptr<py::cpp_function>> cppfunction_handlers;
    std::map<std::string, std::shared_ptr<_SQNativeClosure_>> nativeclosure_handlers;

    SQPythonDict(py::dict dict, HSQUIRRELVM vm) {
        this->vm = vm;
        this->_val = dict;

        cppfunction_handlers["_get"] = std::make_shared<py::cpp_function>([this](TYPE_KEY key) -> PyValue {
            return this->_val[py::str(key)].cast<PyValue>();
        });
        cppfunction_handlers["_set"] = std::make_shared<py::cpp_function>([this](TYPE_KEY key, PyValue value) {
            this->_val.attr("__setitem__")(key, value);
        });
        cppfunction_handlers["_newslot"] = std::make_shared<py::cpp_function>([this](TYPE_KEY key, PyValue value){
            this->_val.attr("__setitem__")(key, value);
        });
        cppfunction_handlers["_delslot"] = std::make_shared<py::cpp_function>([this](TYPE_KEY key) {
            this->_val.attr("__delitem__")(key);
        });

        cppfunction_handlers["pop"] = std::make_shared<py::cpp_function>([this](TYPE_KEY key, PyValue value) -> PyValue {
            return this->_val.attr("pop")(key, value);
        });
        cppfunction_handlers["len"] = std::make_shared<py::cpp_function>([this]() -> PyValue {
            return py::int_(this->_val.size());
        });
        cppfunction_handlers["clear"] = std::make_shared<py::cpp_function>([this]() {
            this->_val.clear();
        });

        for(const auto& [ k, v ]: cppfunction_handlers) {
            nativeclosure_handlers[k] = std::make_shared<_SQNativeClosure_>(_SQNativeClosure_{v, vm});
        }

        _delegate = std::make_shared<_SQTable_>(_SQTable_(vm));
        for(auto pair: nativeclosure_handlers) {
            _delegate->bindFunc(pair.first, pair.second);
        }
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
        SQUserData* ud = _userdata(vm->PopGet());
        ud->SetDelegate(pycontainer->_delegate->pTable);
        ud->_hook = release_SQPythonDict;
        ud->_typetag = &PythonTypeTag::dict;
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
