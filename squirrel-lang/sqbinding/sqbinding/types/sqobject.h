#pragma once

#include "definition.h"

namespace sqbinding {
    namespace detail {
        class ObjectPtr {
            public:
                struct Holder {
                    Holder(::SQObjectPtr& pObject, HSQUIRRELVM vm) : vm(vm) {
                        obj = pObject;
                        sq_addref(vm, &obj);
                    }
                    Holder(::SQObjectPtr&& pObject, HSQUIRRELVM vm) : vm(vm) {
                        obj = pObject;
                        sq_addref(vm, &obj);
                    }
                    ~Holder(){
                        sq_release(vm, &obj);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr obj;
                };

                ObjectPtr(::SQObjectPtr& pObject, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pObject, vm)) {};
                ObjectPtr(::SQObjectPtr&& pObject, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pObject, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return sq_getrefcount(holder->vm, &holder->obj);
                }

                SQObjectType type() {
                    return holder->obj._type;
                }

                SQObjectPtr& operator* () {
                    return (holder->obj);
                }

                std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class ObjectPtr: public detail::ObjectPtr {
            public:
                ObjectPtr(::SQObjectPtr& pObject, HSQUIRRELVM vm): detail::ObjectPtr(pObject, vm) {};
                ObjectPtr(::SQObjectPtr&& pObject, HSQUIRRELVM vm): detail::ObjectPtr(pObject, vm) {};

            PyValue to_python();
            void from_python(PyValue val);

            std::string __str__() {
                return sqobject_to_string(**this);
            }

            std::string __repr__() {
                return "SQObjectPtr(" + sqobject_to_string(**this) + ")";
            }
        };
    }
}
