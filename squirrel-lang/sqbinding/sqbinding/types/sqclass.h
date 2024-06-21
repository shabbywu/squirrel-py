#ifndef _SQBINDING_CLASS_H_
#define _SQBINDING_CLASS_H_

#include "definition.h"
#include "sqiterator.h"
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


// class sqbinding::python::Class : public std::enable_shared_from_this<sqbinding::python::Class>  {
// public:
//     SQObjectPtr handler;
//     SQClass* pClass;
//     HSQUIRRELVM vm = nullptr;
//     bool releaseOnDestroy = false;

//     // link to a existed table in vm stack
//     sqbinding::python::Class (SQClass* pClass, HSQUIRRELVM vm) : pClass(pClass), vm(vm), handler(pClass) {
//         sq_addref(vm, &handler);
//     }

//     sqbinding::python::Class(const sqbinding::python::Class& rhs) {
//         pClass = rhs.pClass;
//         vm = rhs.vm;
//         handler = pClass;
//         sq_addref(vm, &handler);
//     }
//     sqbinding::python::Class& operator=(const sqbinding::python::Class& rhs) {
//         release();
//         pClass = rhs.pClass;
//         vm = rhs.vm;
//         handler = pClass;
//         sq_addref(vm, &handler);
//         return *this;
//     };

//     ~sqbinding::python::Class() {
//         release();
//     }

//     void release() {
//         __check_vmlock(vm)
//         #ifdef TRACE_CONTAINER_GC
//         std::cout << "GC::Release " << __repr__() << " uiRef--=" << pClass -> _uiRef -2 << std::endl;
//         #endif
//         sq_release(vm, &handler);
//         handler.Null();
//     }


//     PyValue get(PyValue key);
//     PyValue set(PyValue key, PyValue val);
//     PyValue getAttributes(PyValue key);
//     PyValue setAttributes(PyValue key, PyValue val);
//     // bindFunc to current class
//     void bindFunc(std::string funcname, py::function func);

//     // Python Interface
//     SQInteger __len__() {
//         return 0;
//         // return pClass->CountUsed();
//     }
//     PyValue __getitem__(PyValue key);
//     PyValue __setitem__(PyValue key, PyValue val);
//     py::list keys();


//     std::string __str__() {
//         return string_format("OT_CLASS: [addr={%p}, ref=%d]", pClass, pClass->_uiRef);
//     }

//     std::string __repr__() {
//         return "SQClass(" + __str__() + ")";
//     }
// };
#endif
