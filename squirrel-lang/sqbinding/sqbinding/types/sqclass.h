#pragma once

#include "sqbinding/common/format.h"
#include "definition.h"
#include "pydict.h"

namespace sqbinding {
    namespace detail {
        class Class: public std::enable_shared_from_this<Class> {
            public:
                struct Holder {
                    Holder(::SQClass* pClass, HSQUIRRELVM vm) : vm(vm) {
                        clazz = pClass;
                        sq_addref(vm, &clazz);
                    }
                    ~Holder(){
                        sq_release(vm, &clazz);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr clazz;
                };

                Class (::SQClass* pClass, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pClass, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pClass() -> _uiRef;
                }

                ::SQClass* pClass() {
                    return _class(holder->clazz);
                }
                std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class Class: public detail::Class, public std::enable_shared_from_this<Class> {
            public:
            // link to a existed table in vm stack
            Class (::SQClass* pClass, HSQUIRRELVM vm): detail::Class(pClass, vm) {}

            PyValue get(PyValue key);
            PyValue set(PyValue key, PyValue val);
            PyValue getAttributes(PyValue key);
            PyValue setAttributes(PyValue key, PyValue val);
            // bindFunc to current class
            void bindFunc(std::string funcname, py::function func);

            // Python Interface
            SQInteger __len__() {
                return 0;
                // return pClass->CountUsed();
            }
            PyValue __getitem__(PyValue key);
            PyValue __setitem__(PyValue key, PyValue val);
            py::list keys();


            std::string __str__() {
                return string_format("OT_CLASS: [addr={%p}, ref=%d]", pClass(), getRefCount());
            }

            std::string __repr__() {
                return "SQClass(" + __str__() + ")";
            }
        };
    }
}
