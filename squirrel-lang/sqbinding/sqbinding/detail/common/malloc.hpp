#pragma once
#include <squirrel.h>
#include "sqbinding/detail/types/sqvm.hpp"

namespace sqbinding {
    namespace detail {
        template <class T, class... Args>
        struct StackObjectHolder {
            StackObjectHolder(Args&... args): instance(new T(args...)) {}
            StackObjectHolder(T* obj): instance(obj) {}
            ~StackObjectHolder() {
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release " << typeid(StackObjectHolder<T, Args...>).name() << std::endl;
                #endif
                if (free_data != nullptr) {
                    free_data(this);
                }
            }
            T* instance;

            T& GetInstance() {
                return *instance;
            }

            void (*free_data)(StackObjectHolder *ptr) = nullptr;
        };

        // construct object on stack
        template <class T, class... Args>
        static std::pair<T*, SQUserData*> make_stack_object(VM vm, Args&... args) {
            using Holder = StackObjectHolder<T, Args...>;
            SQUserPointer ptr = sq_newuserdata(*vm, sizeof(Holder));
            Holder* holder = new(ptr) Holder(args...);

            SQRELEASEHOOK hook = [](SQUserPointer ptr, SQInteger)->SQInteger {
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release " << typeid(T).name() << std::endl;
                #endif
                Holder* obj = (Holder*)ptr;
                obj->~Holder();
                return 0;
            };
            holder->free_data = [](Holder* holder) {
                delete holder->instance;
            };

            sq_setreleasehook(*vm, -1, hook);
            // get userdata in stack top
            SQUserData* ud = _userdata((*vm)->PopGet());
            return std::make_pair(&holder->GetInstance(), ud);
        }

        // copy obj ptr into stack
        template <class T>
        static SQUserData* make_userdata(VM vm, T* obj) {
            using Holder = StackObjectHolder<T>;
            SQUserPointer ptr = sq_newuserdata(*vm, sizeof(Holder));
            Holder* holder = new(ptr) Holder(obj);

            SQRELEASEHOOK hook = [](SQUserPointer ptr, SQInteger)->SQInteger {
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release " << typeid(T).name() << std::endl;
                #endif
                Holder* holder = (Holder*)ptr;
                holder->~Holder();
                return 0;
            };
            sq_setreleasehook(*vm, -1, hook);
            // get userdata in stack top
            SQUserData* ud = _userdata((*vm)->PopGet());
            return ud;
        }
    }
}
