#pragma once

#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqvm.hpp"

namespace sqbinding {
    namespace detail {
        template <class T>
        class Closure;

        template <class Return, class... Args>
        class Closure<Return (Args...)> {
            public:
                struct Holder {
                    Holder(::SQClosure* pClosure, VM vm) : vm(vm) {
                        closure = pClosure;
                        sq_addref(*vm, &closure);
                    }
                    ~Holder(){
                        #ifdef TRACE_CONTAINER_GC
                        std::cout << "GC::Release Closure: " << sqobject_to_string(closure) << std::endl;
                        #endif
                        sq_release(*vm, &closure);
                    }
                    VM vm;
                    SQObjectPtr closure;
                };
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                Closure(::SQClosure* pClosure, VM vm): holder(std::make_shared<Holder>(pClosure, vm)) {};
                ::SQClosure* pClosure() {
                    return _closure(holder->closure);
                }
                SQUnsignedInteger getRefCount() {
                    return pClosure() -> _uiRef;
                }
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                Return operator()(Args... args) {
                    VM& vm = holder->vm;
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        sq_call_setup(vm, holder->closure, pthis, args...);
                    } else {
                        sq_call_setup(vm, holder->closure, (*vm)->_roottable, args...);
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
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->closure;
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
            public:
                struct Holder {
                    Holder(::SQNativeClosure* pNativeClosure, VM vm) : vm(vm) {
                        nativeClosure = pNativeClosure;
                        sq_addref(*vm, &nativeClosure);
                    }
                    ~Holder(){
                        #ifdef TRACE_CONTAINER_GC
                        std::cout << "GC::Release NativeClosure: " << sqobject_to_string(nativeClosure) << std::endl;
                        #endif
                        sq_release(*vm, &nativeClosure);
                    }
                    VM vm;
                    SQObjectPtr nativeClosure;
                };
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                NativeClosure(::SQNativeClosure* pNativeClosure, VM vm): holder(std::make_shared<Holder>(pNativeClosure, vm)) {};
                ::SQNativeClosure* pNativeClosure() {
                    return _nativeclosure(holder->nativeClosure);
                }
                SQUnsignedInteger getRefCount() {
                    return pNativeClosure() -> _uiRef;
                }
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                Return operator()(Args... args) {
                    VM vm = holder->vm;
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        sq_call_setup(vm, holder->nativeClosure, pthis, args...);
                    } else {
                        sq_call_setup(vm, holder->nativeClosure, (*vm)->_roottable, args...);
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
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->nativeClosure;
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
        };
    }
}
