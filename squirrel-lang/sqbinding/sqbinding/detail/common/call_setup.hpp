#pragma once
#include <squirrel.h>
#include <functional>
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
        template <int index, class ReturnType>
        struct load_args;

        template <int index, typename Arg, typename... Args>
        struct load_args<index, std::tuple<Arg, Args...>> {
            static std::tuple<Arg, Args...> load(VM vm) {
                return std::tuple_cat(
                    load_args<index, std::tuple<Arg>>::load(vm),
                    load_args<index + 1, std::tuple<Args...>>::load(vm));
            }
        };

        template <int index, typename Arg>
        struct load_args<index, std::tuple<Arg>> {
            static std::tuple<Arg> load(VM vm) {
                auto arg = generic_stack_get<Arg>(vm, index);
                return std::make_tuple<Arg>(std::forward<Arg>(arg));
            }
        };

        template <int index>
        struct load_args<index, std::tuple<>> {
            static std::tuple<> load(VM vm) {
                return std::make_tuple();
            }
        };
    }

    namespace detail {
        // template <int index, class Return, typename Class, class Arg, class... Args>
        // struct load_args<index, Return (Class::*)(Arg, Args...)>{
        //     using InputType = Return(Class::*)(Arg, Args...);
        //     static auto load(InputType func, VM vm) {
        //         auto arg = generic_stack_get<Arg>(vm, index);
        //         return load_args<index+1, Return, Args...>::load(std::bind(func, arg), vm);
        //     }
        // };

        // template <int index, class Return, typename Class, class Arg>
        // struct load_args<index, Return (Class::*)(Arg)>{
        //     using InputType = Return(Class::*)(Arg);
        //     static auto load(InputType func, VM vm) {
        //         auto arg = generic_stack_get<Arg>(vm, index);
        //         return std::bind(func, arg);
        //     }
        // };

        // template <int index, class Return, typename Class>
        // struct load_args<index, Return(Class::*)()> {
        //     using InputType = Return(Class::*)();
        //     static auto load(InputType func, VM vm) {
        //         return func;
        //     }
        // };
    }
}
