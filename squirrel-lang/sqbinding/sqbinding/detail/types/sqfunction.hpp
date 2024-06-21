#pragma once

#include <squirrel.h>
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"

namespace sqbinding {
    namespace detail {
        template <class T>
        class Closure;

        template <class Return, class... Args>
        class Closure<Return (Args...)> {
            public:
                struct Holder {
                    Holder(::SQClosure* pClosure, HSQUIRRELVM vm) : vm(vm) {
                        closure = pClosure;
                        sq_addref(vm, &closure);
                    }
                    ~Holder(){
                        sq_release(vm, &closure);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr closure;
                };
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                Closure(::SQClosure* pClosure, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pClosure, vm)) {};
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
                    HSQUIRRELVM vm = holder->vm;
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        call_setup(vm, holder->closure, pthis, args...);
                    } else {
                        call_setup(vm, holder->closure, vm->_roottable, args...);
                    }
                    return call<Return>(vm, stack_guard.offset() - 1);
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
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = generic_cast<SQObjectPtr, std::remove_reference_t<TV>>(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->closure;
                    if (!vm->Get(self, key, ret, false, DONT_FALL_BACK)) {
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
                    Holder(::SQNativeClosure* pNativeClosure, HSQUIRRELVM vm) : vm(vm) {
                        nativeClosure = pNativeClosure;
                        sq_addref(vm, &nativeClosure);
                    }
                    ~Holder(){
                        sq_release(vm, &nativeClosure);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr nativeClosure;
                };
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                NativeClosure(::SQNativeClosure* pNativeClosure, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pNativeClosure, vm)) {};
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
                    HSQUIRRELVM vm = holder->vm;
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        call_setup(vm, holder->nativeClosure, pthis, args...);
                    } else {
                        call_setup(vm, holder->nativeClosure, vm->_roottable, args...);
                    }
                    return call<Return>(vm, stack_guard.offset() - 1);
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
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = generic_cast<SQObjectPtr, std::remove_reference_t<TV>>(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->nativeClosure;
                    if (!vm->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
        };
    }
}