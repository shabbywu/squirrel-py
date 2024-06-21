#pragma once
#include <squirrel.h>
#include "cast.hpp"

namespace sqbinding {
    namespace detail {
        class stack_guard {
            public:
                stack_guard(HSQUIRRELVM v) {
                    vm = v;
                    top = sq_gettop(vm);
                }
                ~stack_guard() {
                    sq_settop(vm, top);
                }
            private:
                HSQUIRRELVM vm;
                SQInteger top;

            public:
                int offset() {
                    return sq_gettop(vm) - top;
                }
        };

        template <class Arg> inline
        void generic_stack_push(HSQUIRRELVM vm, Arg arg) {
            sq_pushobject(vm, arg);
        }

        template <class Return> inline
        Return generic_stack_get(HSQUIRRELVM vm, SQInteger index) {
            HSQOBJECT ref;
            sq_getstackobj(vm, index, &ref);
            return generic_cast<HSQOBJECT, Return>(vm, ref);
        }

    }
}
