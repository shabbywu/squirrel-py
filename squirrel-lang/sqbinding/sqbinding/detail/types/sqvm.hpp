#pragma once
#include "sqbinding/detail/common/debug.hpp"
#include "sqbinding/detail/sqdefinition.hpp"
#include <memory>

namespace sqbinding {
namespace detail {
class VM {
  public:
    struct Holder {
        Holder(HSQUIRRELVM vm, bool should_close = false) : vm(vm), should_close(should_close) {
        }
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
        HSQUIRRELVM vm = nullptr;
        bool should_close;
    };

  public:
    std::shared_ptr<Holder> holder;
    static int ref_count;

  public:
    VM() = default;
    VM(HSQUIRRELVM vm, bool should_close = false) : holder(std::make_shared<Holder>(vm, should_close)) {
        ref_count++;
    }

    VM(const VM &other) {
        this->holder = other.holder;
        ref_count++;
        std::cout << "VM(const): ref_count = " << ref_count << std::endl;
    };

    VM &operator=(const VM &other) {
        this->holder = other.holder;
        ref_count++;
        std::cout << "VM=: ref_count = " << ref_count << std::endl;
        return *this;
    };
    ~VM() {
        if (holder) {
            ref_count--;
        }
        std::cout << "~VM: ref_count = " << ref_count << std::endl;
    }

  public:
    HSQUIRRELVM &vm() {
        return holder->vm;
    }
    SQObjectPtr &roottable() {
        return holder->vm->_roottable;
    }
    HSQUIRRELVM &operator*() {
        return holder->vm;
    }
};
int VM::ref_count = 0;

class WeakVM: public VM {
  public:
    std::weak_ptr<VM::Holder> holder;
    WeakVM(const VM &other) {
        this->holder = other.holder;
    };

    WeakVM &operator=(const VM &other) = delete;

  public:
    HSQUIRRELVM &vm() {
        return holder.lock()->vm;
    }
    SQObjectPtr &roottable() {
        return holder.lock()->vm->_roottable;
    }
    HSQUIRRELVM &operator*() {
        return holder.lock()->vm;
    }
};

} // namespace detail
} // namespace sqbinding
