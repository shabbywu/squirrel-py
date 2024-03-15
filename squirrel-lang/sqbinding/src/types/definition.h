#ifndef _SQBINDING_DEFINITION_H_
#define _SQBINDING_DEFINITION_H_

#include "../sqheader.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <variant>
#include <string>
#include <iostream>


namespace py = pybind11;

typedef std::variant<py::none, py::int_, float, py::bool_, std::string, py::list, py::dict, py::function, py::type, std::shared_ptr<_SQString_>, std::shared_ptr<_SQArray_>, std::shared_ptr<_SQTable_>, std::shared_ptr<_SQClosure_>, std::shared_ptr<_SQNativeClosure_>, std::shared_ptr<_SQClass_>, std::shared_ptr<_SQInstance_>, std::shared_ptr<ArrayIterator>, std::shared_ptr<TableIterator>, py::object> PyValue;


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
