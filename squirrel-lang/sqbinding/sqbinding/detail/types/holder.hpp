#pragma once
#include "sqbinding/detail/cast/cast_string.hpp"
#include "squirrel.h"
#include "sqvm.hpp"
#include <string>

namespace sqbinding {
namespace detail {
template <class Type> class SQObjectPtrHolder {
  public:
    SQObjectPtrHolder(Type obj, VM vm) : ptr(obj), vm(vm) {
        sq_addref(*vm, &ptr);
    }


    ~SQObjectPtrHolder() {
#ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << typeid(Type).name() << ": " << sqobject_to_string(ptr) << std::endl;
#endif
        sq_release(*vm, &ptr);
    }

  public:
    VM vm;
    SQObjectPtr ptr;

  public:
    HSQUIRRELVM &GetSQVM() {
        return *vm;
    }
    VM &GetVM() {
        return vm;
    }
    SQObjectPtr &GetSQObjectPtr() {
        return ptr;
    }
};
} // namespace detail
} // namespace sqbinding
