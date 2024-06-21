#pragma once
#include <squirrel.h>
#include "cast.h"
#include "stack_operation.h"

namespace sqbinding {
    namespace detail {
        inline
        void call_setup_arg(HSQUIRRELVM vm) {}

        template <class Arg, class... Args> inline
        void call_setup_arg(HSQUIRRELVM vm, Arg head, Args... tail) {
            generic_stack_push(vm, head);
            call_setup_arg(vm, tail...);
        }

        template <class... Args> inline
        void call_setup(HSQUIRRELVM vm, const HSQOBJECT& closure, const HSQOBJECT& table, Args... args) {
            sq_pushobject(vm, closure);
            sq_pushobject(vm, table);
            call_setup_arg(vm, args...);
        }

        template <class Return> inline
        Return call(HSQUIRRELVM vm, int params_count) {
            if (SQ_FAILED(sq_call(vm, params_count, SQTrue, SQTrue))) {
                const SQChar* sqErr;
                sq_getlasterror(vm);
                if (sq_gettype(vm, -1) == OT_NULL) {
                    throw std::runtime_error("unknown error");
                }
                sq_tostring(vm, -1);
                sq_getstring(vm, -1, &sqErr);
                throw std::runtime_error(std::string(sqErr));
            } else {
                return generic_stack_get<Return>(vm, -1);
            }
        }
    }
}
