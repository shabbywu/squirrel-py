#pragma once
#include "sqbinding/detail/types/sqvm.hpp"
#include "stack_operation.hpp"
#include <functional>
#include <squirrel.h>

namespace sqbinding {
namespace detail {
inline void sq_call_setup_arg(VM vm) {
}

template <class Arg, class... Args> inline void sq_call_setup_arg(VM vm, Arg&& head, Args&&... tail) {
    generic_stack_push(vm, std::forward<Arg>(head));
    sq_call_setup_arg(vm, tail...);
}

template <class... Args>
inline void sq_call_setup(VM vm, const HSQOBJECT &closure, const HSQOBJECT &table, Args&&... args) {
    sq_pushobject(*vm, closure);
    sq_pushobject(*vm, table);
    sq_call_setup_arg(vm, args...);
}

template <class Return> inline Return sq_call(VM vm, int params_count) {
    if (SQ_FAILED(::sq_call(*vm, params_count, SQTrue, SQTrue))) {
        const SQChar *sqErr;
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
} // namespace detail

namespace detail {
// load_args will load args from sqvm stack into cpp memory
template <int index, class ReturnType> struct load_args;

// default implement, will load args as std::tuple
//
// Example to call cpp_func with args in sqvm stack
// template<class Return, class... Args>
// Return call_func(Return(*func)(Args...)) {
//    std::tuple<Args...> args = load_args<Args...>(vm);
//    return std::apply(func, args);
// }
template <int index, typename Arg, typename... Args> struct load_args<index, std::tuple<Arg, Args...>> {
    static std::tuple<Arg, Args...> load(VM vm) {
        return std::tuple_cat(load_args<index, std::tuple<Arg>>::load(vm),
                              load_args<index + 1, std::tuple<Args...>>::load(vm));
    }
};

template <int index, typename Arg> struct load_args<index, std::tuple<Arg>> {
    static std::tuple<Arg> load(VM vm) {
        auto arg = generic_stack_get<Arg>(vm, index);
        return std::make_tuple<Arg>(std::forward<Arg>(arg));
    }
};

template <int index> struct load_args<index, std::tuple<>> {
    static std::tuple<> load(VM vm) {
        return std::make_tuple();
    }
};
} // namespace detail
} // namespace sqbinding
