#pragma once
#include <squirrel.h>
#include "cast_def.hpp"
#include "sqbinding/detail/types/sqvm.hpp"

namespace sqbinding {
    namespace detail {
        class stack_guard {
            public:
                stack_guard(VM v) {
                    vm = v;
                    top = sq_gettop(*vm);
                }
                ~stack_guard() {
                    sq_settop(*vm, top);
                }
            private:
                VM vm;
                SQInteger top;

            public:
                int offset() {
                    return sq_gettop(*vm) - top;
                }
        };

        template <class Arg> inline
        void generic_stack_push(VM vm, Arg arg) {
            sq_pushobject(*vm, arg);
        }

        template <class Return> inline
        Return generic_stack_get(VM vm, SQInteger index) {
            HSQOBJECT ref;
            sq_getstackobj(*vm, index, &ref);
            return GenericCast<Return(HSQOBJECT&)>::cast(vm, ref);
        }

    }
}
