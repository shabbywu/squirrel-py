#pragma once
#include "sqvm.hpp"

namespace sqbinding {
    namespace detail {
        class ObjectPtr {
            public:
                struct Holder {
                    Holder(::SQObjectPtr& pObject, VM vm) : vm(vm) {
                        obj = pObject;
                        sq_addref(*vm, &obj);
                    }
                    Holder(::SQObjectPtr&& pObject, VM vm) : vm(vm) {
                        obj = pObject;
                        sq_addref(*vm, &obj);
                    }
                    ~Holder(){
                        sq_release(*vm, &obj);
                    }
                    VM vm;
                    SQObjectPtr obj;
                };

                ObjectPtr(::SQObjectPtr& pObject, VM vm): holder(std::make_shared<Holder>(pObject, vm)) {};
                ObjectPtr(::SQObjectPtr&& pObject, VM vm): holder(std::make_shared<Holder>(pObject, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return sq_getrefcount(*holder->vm, &holder->obj);
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
}
