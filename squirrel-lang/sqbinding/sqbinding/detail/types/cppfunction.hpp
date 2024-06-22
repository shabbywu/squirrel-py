#pragma once
#include <functional>
#include <memory>
#include "sqvm.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/malloc.hpp"

namespace sqbinding {
    namespace detail {
        template <class T>
        class CppFunction;

        template <class Return, class...Args>
        class CppFunction<Return (Args...)> {
            public:
                struct Holder {
                    Holder(std::function<Return(Args...)>& func, detail::VM vm): func(func), vm(vm){};

                    detail::VM vm;
                    std::function<Return(Args...)> func;
                };
            public:
                CppFunction(std::function<Return(Args...)>& func, detail::VM vm): holder(std::make_shared<Holder>(func, vm)) {}
            public:
                std::shared_ptr<std::function<Return(Args...)>> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                Return operator()(Args... args) {
                    VM& vm = holder->vm;
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        call_setup(vm, holder->closure, pthis, args...);
                    } else {
                        call_setup(vm, holder->closure, (*vm)->_roottable, args...);
                    }
                    return call<Return>(vm, stack_guard.offset() - 1);
                }
            public:
                static SQUserData* Create(std::function<Return(Args...)>& func, detail::VM vm) {
                    // new userdata to store py::function
                    auto result = detail::make_stack_object<CppFunction, std::function<Return(Args...)>, detail::VM>(vm, func, vm);
                    auto pycontainer = result.first;
                    auto ud = result.second;
                    ud->SetDelegate(pycontainer->_delegate->pTable());
                    ud->_typetag = typeid(Return(Args...)).hash_code();
                    return ud;
                }

                static SQUserData* Create(Return(*func)(Args...) , detail::VM vm) {
                    // new userdata to store py::function
                    auto result = detail::make_stack_object<CppFunction, std::function<Return(Args...)>, detail::VM>(vm, func, vm);
                    auto pycontainer = result.first;
                    auto ud = result.second;
                    ud->SetDelegate(pycontainer->_delegate->pTable());
                    ud->_typetag = typeid(Return(Args...)).hash_code();
                    return ud;
                }
        };
    }
}
