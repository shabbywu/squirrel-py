#ifndef _SQBINDING_DEFINITION_H_
#define _SQBINDING_DEFINITION_H_

#include "../sqheader.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <variant>
#include <map>
#include <string>
#include <iostream>


namespace py = pybind11;

typedef std::variant<py::none, py::int_, py::float_, py::bool_, std::string, py::list, py::dict, py::function, py::type, std::shared_ptr<_SQString_>, std::shared_ptr<_SQArray_>, std::shared_ptr<_SQTable_>, std::shared_ptr<_SQClosure_>, std::shared_ptr<_SQNativeClosure_>, std::shared_ptr<_SQClass_>, std::shared_ptr<_SQInstance_>, std::shared_ptr<ArrayIterator>, std::shared_ptr<TableIterator>, py::object> PyValue;


namespace vmlock {
    static std::map<uintptr_t, int> _vm_handles;
    static void register_vm_handle(HSQUIRRELVM vm) {
        auto k = reinterpret_cast<uintptr_t>(vm);
        if (!_vm_handles.contains(k)) {
            _vm_handles[k] = 1;
            return;
        }
        _vm_handles[k] = _vm_handles[k] + 1;
    }
    static void unregister_vm_handle(HSQUIRRELVM vm) {
        auto k = reinterpret_cast<uintptr_t>(vm);
        if (!_vm_handles.contains(k)) {
            return;
        }
        auto v = _vm_handles[k] - 1;
        if (v > 0) {
             _vm_handles[k] = v;
        } else {
            _vm_handles.erase(k);
        }
    }
    static bool contain_vm_handle(HSQUIRRELVM vm) {
        auto k = reinterpret_cast<uintptr_t>(vm);
        if (!_vm_handles.contains(k)) {return false;}
        return _vm_handles[k] > 0;
    }
    #ifdef TRACE_CONTAINER_GC
    static void print_vm_handles() {
        for(auto iter = _vm_handles.begin(); iter != _vm_handles.end(); iter ++) {
            std::cout << "key: " << iter->first << " value: " << iter->second << std::endl;
        }
    }
    #endif
}


#ifdef TRACE_CONTAINER_GC
#define __check_vmlock(vm)\
if (!vmlock::contain_vm_handle(vm)) {\
    std::cout << "GC::Release error, vm is closed" << std::endl;\
    vmlock::print_vm_handles();\
    return;\
}
#else
#define __check_vmlock(vm)\
if (!vmlock::contain_vm_handle(vm)) {\
    return;\
}
#endif


template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}
std::string sqobject_to_string(SQObjectPtr&);

PyValue sqobject_topython(SQObjectPtr& object, HSQUIRRELVM vm);
SQObjectPtr pyvalue_tosqobject(PyValue object, HSQUIRRELVM vm);
PyValue pyobject_topyvalue(py::object object);
#endif
