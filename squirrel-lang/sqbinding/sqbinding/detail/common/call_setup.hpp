#pragma once
#include <squirrel.h>
#include "cast_def.hpp"
#include "stack_operation.hpp"
#include "sqbinding/detail/types/sqvm.hpp"

namespace sqbinding {
    namespace detail {
        inline
        void sq_call_setup_arg(VM vm) {}

        template <class Arg, class... Args> inline
        void sq_call_setup_arg(VM vm, Arg head, Args... tail) {
            generic_stack_push(vm, head);
            sq_call_setup_arg(vm, tail...);
        }

        template <class... Args> inline
        void sq_call_setup(VM vm, const HSQOBJECT& closure, const HSQOBJECT& table, Args... args) {
            sq_pushobject(*vm, closure);
            sq_pushobject(*vm, table);
            sq_call_setup_arg(vm, args...);
        }

        template <class Return> inline
        Return sq_call(VM vm, int params_count) {
            if (SQ_FAILED(::sq_call(*vm, params_count, SQTrue, SQTrue))) {
                const SQChar* sqErr;
                sq_getlasterror(*vm);
                if (sq_gettype(*vm, -1) == OT_NULL) {
                    throw std::runtime_error("unknown error");
                }
                sq_tostring(*vm, -1);
                sq_getstring(*vm, -1, &sqErr);
                throw std::runtime_error(std::string(sqErr));
            } else {
                return generic_stack_get<Return>(vm, -1);
            }
        }
    }

    namespace detail {
        template <int index, typename Func>
        struct load_args;

        template <int index, class Return, class Arg, class... Args>
        struct load_args<index, Return(Arg, Args...)>{
            static std::function<Return(Args...)> load(std::function<Return(Arg, Args...)> func, VM vm) {
                auto arg = generic_stack_get<Arg>(vm, index);
                return load_args<index+1, Return, Args...>::load(std::bind(func, arg), vm);
            }
        };

        template <int index, class Return, class Arg>
        struct load_args<index, Return(Arg)>{
            static std::function<Return()> load(std::function<Return(Arg)> func, VM vm) {
                auto arg = generic_stack_get<Arg>(vm, index);
                return std::bind(func, arg);
            }
        };

        template <int index, class Return>
        struct load_args<index, Return()> {
            static std::function<Return()> load(std::function<Return()> func, VM vm) {
                return func;
            }
        };
    }
}
