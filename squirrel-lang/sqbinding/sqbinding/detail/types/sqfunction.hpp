#pragma once

#include "sqbinding/detail/sqdefinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"
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
                SQOBJECTPTR_GETTER_TEMPLATE
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
                SQOBJECTPTR_GETTER_TEMPLATE
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
                template<class Wrapper, class Func>
                static std::shared_ptr<NativeClosure> Create(Func&& func, detail::VM vm, SQFUNCTION caller) {
                    auto pair = detail::make_stack_object<Wrapper>(vm, func);
                    std::shared_ptr<NativeClosure> closure = std::make_shared<NativeClosure>(SQNativeClosure::Create(_ss(*vm), caller, 1), vm);
                    closure->pNativeClosure()->_outervalues[0] = pair.second;
                    closure->pNativeClosure()->_nparamscheck = 0;
                    return closure;
                }
        };
    }

    // cast to SQObjectPtr
    namespace detail {

        template <class Return, class... Args>
        class GenericCast<SQObjectPtr(std::shared_ptr<NativeClosure<Return(Args...)>>&)> {
            public:
            static SQObjectPtr cast(VM vm, std::shared_ptr<NativeClosure<Return(Args...)>>& obj) {
                #ifdef TRACE_OBJECT_CAST
                std::cout << "[TRACING] cast " << typeid(NativeClosure<Return(Args...)>&).name() << " to SQObjectPtr" << std::endl;
                #endif
                return SQObjectPtr(obj->pNativeClosure());
            }
        };
    }
}
