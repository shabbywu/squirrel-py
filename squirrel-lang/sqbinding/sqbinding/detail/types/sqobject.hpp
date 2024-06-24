#pragma once
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqvm.hpp"
#include "holder.hpp"

namespace sqbinding {
    namespace detail {
        class ObjectPtr {
            public:
            using Holder = SQObjectPtrHolder<::SQObjectPtr>;
            public:
                ObjectPtr(::SQObjectPtr& pObject, VM vm): holder(std::make_shared<Holder>(pObject, vm)) {};
                ObjectPtr(::SQObjectPtr&& pObject, VM vm): holder(std::make_shared<Holder>(pObject, vm)) {};
            public:
                std::shared_ptr<Holder> holder;
            public:
                SQUnsignedInteger getRefCount() {
                    return sq_getrefcount(holder->GetSQVM(), &holder->GetSQObjectPtr());
                }

                SQObjectType type() {
                    return holder->GetSQObjectPtr()._type;
                }

                SQObjectPtr& operator* () {
                    return (holder->GetSQObjectPtr());
                }
        };
    }
}
