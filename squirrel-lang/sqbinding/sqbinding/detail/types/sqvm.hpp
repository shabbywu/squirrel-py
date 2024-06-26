#pragma once
#include <memory>
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/debug.hpp"

namespace sqbinding {
    namespace detail {
        class VM {
              class Holder {
                    public:
                        Holder(HSQUIRRELVM vm, bool should_close = false) : vm(vm), should_close(should_close) {}
                        ~Holder() {
                            if (should_close) {
                                #ifdef TRACE_CONTAINER_GC
                                std::cout << "GC::Release HSQUIRRELVM Holder(closing): " << vm << std::endl;
                                #endif
                                detail::time_guard time_guard;
                                sq_settop(vm, 0);
                                sq_collectgarbage(vm);
                                sq_close(vm);
                            } else {
                                #ifdef TRACE_CONTAINER_GC
                                std::cout << "GC::Release HSQUIRRELVM Holder: " << vm << std::endl;
                                #endif
                            }
                        }
                    public:
                        HSQUIRRELVM vm = nullptr;
                        bool should_close;
                };
                public:
                    std::shared_ptr<Holder> holder;
                public:
                    VM() = default;
                    VM(HSQUIRRELVM vm, bool should_close = false): holder(std::make_shared<Holder>(vm, should_close)) {}
                public:
                    HSQUIRRELVM& vm() {
                        return holder->vm;
                    }
                    SQObjectPtr& roottable() {
                        return holder->vm->_roottable;
                    }
                    HSQUIRRELVM& operator*(){
                        return holder->vm;
                    }
        };
    }
}
