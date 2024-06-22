#pragma once
#include <squirrel.h>
#include "sqbinding/detail/types/sqvm.hpp"

namespace sqbinding {
    namespace detail {
        // construct object on stack
        template <class T, class... Args>
        static std::pair<T*, SQUserData*> make_stack_object(VM vm, Args&... args) {
            struct Holder {
                Holder(Args&... args): instance(std::make_shared<T>(args...)) {}
                std::shared_ptr<T> instance;
            };
            SQUserPointer ptr = sq_newuserdata(*vm, sizeof(Holder));
            Holder* obj = new(ptr) Holder(args...);

            SQRELEASEHOOK hook = [](SQUserPointer ptr, SQInteger)->SQInteger {
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release " << typeid(T).name() << std::endl;
                #endif
                Holder* obj = (Holder*)ptr;
                obj->~Holder();
                return 0;
            };
            sq_setreleasehook(*vm, -1, hook);
            // get userdata in stack top
            SQUserData* ud = _userdata((*vm)->PopGet());
            return std::make_pair(obj->instance.get(), ud);
        }
    }
}
