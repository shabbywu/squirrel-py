#pragma once
#include <squirrel.h>
#include <sqvm.h>
#include <memory>
#include "sqbinding/detail/common/debug.hpp"

namespace sqbinding {
    namespace detail {
        class VM {
              class Holder {
                    public:
                        Holder(HSQUIRRELVM vm, bool should_close = false) : vm(vm), should_close(should_close) {}
                        ~Holder() {
                            if (vm == nullptr) {
                                return;
                            }
                            #ifdef TRACE_CONTAINER_GC
                            std::cout << "GC::Release VM: " << vm << std::endl;
                            #endif
                            if (should_close) {
                                detail::time_guard time_guard;
                                sq_settop(vm, 0);
                                sq_collectgarbage(vm);
                                sq_close(vm);
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
                    public:
                        SQObjectPtr& roottable() {
                            return holder->vm->_roottable;
                        }
                    public:
                        HSQUIRRELVM& operator*(){
                            return holder->vm;
                        }
        };
    }
}
