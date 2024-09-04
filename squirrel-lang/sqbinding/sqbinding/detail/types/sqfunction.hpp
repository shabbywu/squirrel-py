#pragma once

#include "holder.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/sqdefinition.hpp"
#include "sqvm.hpp"

namespace sqbinding {
namespace detail {
template <class T> class Closure;

template <class Return, class... Args> class Closure<Return(Args...)> : public ClosureBase {
    using Holder = SQObjectPtrHolder<::SQClosure *>;
    using FuncSignautre = Return(Args...);

  public:
    std::shared_ptr<Holder> holder;
    SQObjectPtr pthis; // 'this' pointer for sq_call
  public:
    Closure(::SQClosure *pClosure, VM vm) : holder(std::make_shared<Holder>(pClosure, vm)) {};

  public:
    std::shared_ptr<Closure<Return(Args...)>> to_ptr() {
        auto ptr = std::make_shared<Closure<Return(Args...)>>(pClosure(), holder->GetVM());
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            ptr->pthis = pthis;
        }
        return ptr;
    };

    ::SQClosure *pClosure() {
        return _closure(holder->GetSQObjectPtr());
    }
    SQUnsignedInteger getRefCount() {
        return pClosure()->_uiRef;
    }

  public:
    virtual void bindThis(SQObjectPtr &pthis) {
        this->pthis = pthis;
    }
    virtual void bindThis(SQObjectPtr &&pthis) {
        this->pthis = pthis;
    }

  public:
    Return operator()(Args... args) {
        VM &vm = holder->GetVM();
        stack_guard stack_guard(vm);
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args...);
        } else {
            sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args...);
        }
        return sq_call<Return>(vm, stack_guard.offset() - 1);
    }

  public:
    std::string to_string() {
        return string_format("OT_CLOSURE: [addr={%p}, ref=%d]", pClosure(), getRefCount());
    }

  public:
    SQOBJECTPTR_GETTER_TEMPLATE
  protected:
    bool get(SQObjectPtr &key, SQObjectPtr &ret) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
            return false;
        }
        return true;
    }
};
} // namespace detail

} // namespace sqbinding

namespace sqbinding {
namespace detail {
class NativeClosureBase : public ClosureBase {
  protected:
    using Holder = SQObjectPtrHolder<::SQNativeClosure *>;

  public:
    NativeClosureBase(::SQNativeClosure *pNativeClosure, VM vm)
        : holder(std::make_shared<Holder>(pNativeClosure, vm)) {};

  public:
    std::shared_ptr<Holder> holder;
    SQObjectPtr pthis; // 'this' pointer for sq_call

  public:
    ::SQNativeClosure *pNativeClosure() {
        return _nativeclosure(holder->GetSQObjectPtr());
    }
    SQUnsignedInteger getRefCount() {
        return pNativeClosure()->_uiRef;
    }

  public:
    virtual void bindThis(SQObjectPtr &pthis) {
        this->pthis = pthis;
    }
    virtual void bindThis(SQObjectPtr &&pthis) {
        this->pthis = pthis;
    }

  public:
    std::string to_string() {
        return string_format("OT_NATIVECLOSURE: [addr={%p}, ref=%d]", pNativeClosure(), getRefCount());
    }

  public:
    SQOBJECTPTR_GETTER_TEMPLATE
  protected:
    bool get(SQObjectPtr &key, SQObjectPtr &ret) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
            return false;
        }
        return true;
    }
};

template <typename Func> class NativeClosure;

template <class Return, class... Args> class NativeClosure<Return(Args...)> : public NativeClosureBase {
  public:
    using FuncSignautre = Return(Args...);

  public:
    NativeClosure(::SQNativeClosure *pNativeClosure, VM vm) : NativeClosureBase(pNativeClosure, vm) {};
    std::shared_ptr<NativeClosure> to_ptr() {
        auto ptr = std::make_shared<NativeClosure>(pNativeClosure(), holder->GetVM());
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            ptr->pthis = pthis;
        }
        return ptr;
    };

  public:
    Return operator()(Args... args) {
        VM vm = holder->GetVM();
        stack_guard stack_guard(vm);
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args...);
        } else {
            sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args...);
        }
        return sq_call<Return>(vm, stack_guard.offset() - 1);
    }

  public:
    static inline auto Create(std::shared_ptr<generic_function> func, detail::VM vm) {
        auto ud = detail::make_userdata(vm, func);
        NativeClosure closure(SQNativeClosure::Create(_ss(*vm), func->get_static_caller(), 1), vm);
        closure.pNativeClosure()->_outervalues[0] = ud;
        closure.pNativeClosure()->_nparamscheck = 0;
        return closure;
    }
};

template <class Return, class... Args> class NativeClosure<Return(Args...) const> : public NativeClosureBase {
  public:
    using FuncSignautre = Return(Args...);

  public:
    NativeClosure(::SQNativeClosure *pNativeClosure, VM vm) : NativeClosureBase(pNativeClosure, vm) {};
    std::shared_ptr<NativeClosure> to_ptr() {
        auto ptr = std::make_shared<NativeClosure>(pNativeClosure(), holder->GetVM());
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            ptr->pthis = pthis;
        }
        return ptr;
    };

  public:
    Return operator()(Args... args) {
        VM vm = holder->GetVM();
        stack_guard stack_guard(vm);
        if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
            sq_call_setup(vm, holder->GetSQObjectPtr(), pthis, args...);
        } else {
            sq_call_setup(vm, holder->GetSQObjectPtr(), (*vm)->_roottable, args...);
        }
        return sq_call<Return>(vm, stack_guard.offset() - 1);
    }

  public:
    static inline auto Create(std::shared_ptr<generic_function> func, detail::VM vm) {
        auto ud = detail::make_userdata(vm, func);
        NativeClosure closure(SQNativeClosure::Create(_ss(*vm), func->get_static_caller(), 1), vm);
        closure.pNativeClosure()->_outervalues[0] = ud;
        closure.pNativeClosure()->_nparamscheck = 0;
        return closure;
    }
};

template <typename Func> static auto CreateNativeClosure(Func &&func, detail::VM vm) {
    auto wrapper = to_cpp_function(std::forward<Func>(func));
    return detail::NativeClosure<detail::function_signature_t<Func>>::Create(wrapper, vm);
}

} // namespace detail

} // namespace sqbinding
