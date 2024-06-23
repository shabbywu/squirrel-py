#pragma once

#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqvm.hpp"
#include "holder.hpp"

namespace sqbinding {
    namespace detail {
        template <class T>
        class Closure;

        template <class Return, class... Args>
        class Closure<Return (Args...)> {
            using Holder = SQObjectPtrHolder<::SQClosure*>;
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                Closure(::SQClosure* pClosure, VM vm): holder(std::make_shared<Holder>(pClosure, vm)) {};
                ::SQClosure* pClosure() {
                    return _closure(holder->GetSQObjectPtr());
                }
                SQUnsignedInteger getRefCount() {
                    return pClosure() -> _uiRef;
                }
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                Return operator()(Args... args) {
                    VM& vm = holder->GetVM();
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args...);
                    } else {
                        sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args...);
                    }
                    return sq_call<Return>(vm, stack_guard.offset() - 1);
                }
            public:
                std::string to_string() {
                    return string_format("OT_CLOSURE: [addr={%p}, ref=%d]", pClosure(), getRefCount());
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    VM& vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
        };
    }
}

namespace sqbinding {
    namespace detail {
        template <class T>
        class NativeClosure;

        template <class Return, class... Args>
        class NativeClosure<Return (Args...)> {
            using Holder = SQObjectPtrHolder<::SQNativeClosure*>;
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                NativeClosure(::SQNativeClosure* pNativeClosure, VM vm): holder(std::make_shared<Holder>(pNativeClosure, vm)) {};
                ::SQNativeClosure* pNativeClosure() {
                    return _nativeclosure(holder->GetSQObjectPtr());
                }
                SQUnsignedInteger getRefCount() {
                    return pNativeClosure() -> _uiRef;
                }
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                Return operator()(Args... args) {
                    VM vm = holder->GetVM();
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args...);
                    } else {
                        sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args...);
                    }
                    return sq_call<Return>(vm, stack_guard.offset() - 1);
                }

            public:
                std::string to_string() {
                    return string_format("OT_NATIVECLOSURE: [addr={%p}, ref=%d]", pNativeClosure(), getRefCount());
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    VM vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM vm = holder->GetVM();
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
        };
    }
}
