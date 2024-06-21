#pragma once

#include "definition.h"
#include "sqiterator.h"
#include "pydict.h"


namespace sqbinding {
    namespace detail {
        class Instance: public std::enable_shared_from_this<Instance> {
            public:
                struct Holder {
                    Holder(::SQInstance* pInstance, HSQUIRRELVM vm) : vm(vm) {
                        instance = pInstance;
                        sq_addref(vm, &instance);
                    }
                    ~Holder(){
                        sq_release(vm, &instance);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr instance;
                };

                Instance (::SQInstance* pInstance, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pInstance, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pInstance() -> _uiRef;
                }

                ::SQInstance* pInstance() {
                    return _instance(holder->instance);
                }
                std::shared_ptr<Holder> holder;
        };
    }
    namespace python {
        class Instance: public detail::Instance, std::enable_shared_from_this<Instance>{
            public:
            // link to a existed pInstance in vm stack
            Instance (::SQInstance* pInstance, HSQUIRRELVM vm): detail::Instance(pInstance, vm) {};

            PyValue get(PyValue key);
            PyValue set(PyValue key, PyValue val);
            PyValue getAttributes(PyValue key);
            PyValue setAttributes(PyValue key, PyValue val);
            // bindFunc to current instance
            void bindFunc(std::string funcname, py::function func);

            // Python Interface
            PyValue __getitem__(PyValue key);
            PyValue __setitem__(PyValue key, PyValue val);
            py::list keys();

            std::string __str__() {
                return string_format("OT_INSTANCE: [addr={%p}, ref=%d]", pInstance(), getRefCount());
            }

            std::string __repr__() {
                return "SQInstance(" + __str__() + ")";
            }
        };
    }
}
