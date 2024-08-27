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

namespace detail {
// cast SQObject to Closure
template <class Return, class... Args> class GenericCast<Closure<Return(Args...)>(HSQOBJECT &)> {
  public:
    static Closure<Return(Args...)> cast(VM vm, HSQOBJECT &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast HSQOBJECT to " << typeid(Closure<Return(Args...)> &).name() << std::endl;
#endif
        return Closure<Return(Args...)>(_closure(obj), vm);
    }
};

template <class Return, class... Args> class GenericCast<Closure<Return(Args...)>(HSQOBJECT &&)> {
  public:
    static Closure<Return(Args...)> cast(VM vm, HSQOBJECT &&obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast HSQOBJECT to " << typeid(Closure<Return(Args...)> &).name() << std::endl;
#endif
        return Closure<Return(Args...)>(_closure(obj), vm);
    }
};

// cast SQObjectPtr to Closure
template <class Return, class... Args> class GenericCast<Closure<Return(Args...)>(SQObjectPtr &)> {
  public:
    static Closure<Return(Args...)> cast(VM vm, SQObjectPtr &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(Closure<Return(Args...)> &).name() << std::endl;
#endif
        return Closure<Return(Args...)>(_closure(obj), vm);
    }
};

template <class Return, class... Args> class GenericCast<Closure<Return(Args...)>(SQObjectPtr &&)> {
  public:
    static Closure<Return(Args...)> cast(VM vm, SQObjectPtr &&obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(Closure<Return(Args...)> &).name() << std::endl;
#endif
        return Closure<Return(Args...)>(_closure(obj), vm);
    }
};

// cast Closure to SQObjectPtr
template <class Return, class... Args> class GenericCast<SQObjectPtr(std::shared_ptr<Closure<Return(Args...)>> &)> {
  public:
    static SQObjectPtr cast(VM vm, std::shared_ptr<Closure<Return(Args...)>> &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast " << typeid(Closure<Return(Args...)> &).name() << " to SQObjectPtr" << std::endl;
#endif
        return SQObjectPtr(obj->pClosure());
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
    template <class Wrapper, class Func>
    static std::shared_ptr<NativeClosure> Create(Func &&func, detail::VM vm, SQFUNCTION caller) {
        auto pair = detail::make_stack_object<Wrapper>(vm, func);
        std::shared_ptr<NativeClosure> closure =
            std::make_shared<NativeClosure>(SQNativeClosure::Create(_ss(*vm), caller, 1), vm);
        closure->pNativeClosure()->_outervalues[0] = pair.second;
        closure->pNativeClosure()->_nparamscheck = 0;
        return closure;
    }
};

template <class Return, class... Args> class NativeClosure<Return(Args...) const> : public NativeClosureBase {
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
    template <class Wrapper, class Func>
    static std::shared_ptr<NativeClosure> Create(Func &&func, detail::VM vm, SQFUNCTION caller) {
        auto pair = detail::make_stack_object<Wrapper>(vm, func);
        std::shared_ptr<NativeClosure> closure =
            std::make_shared<NativeClosure>(SQNativeClosure::Create(_ss(*vm), caller, 1), vm);
        closure->pNativeClosure()->_outervalues[0] = pair.second;
        closure->pNativeClosure()->_nparamscheck = 0;
        return closure;
    }
};

template <typename Func> static auto CreateNativeClosure(Func &&func, detail::VM vm) {
    if (detail::function_traits<Func>::value == detail::CppFuntionType::LambdaLike ||
        detail::function_traits<Func>::value == detail::CppFuntionType::VanillaFunctionPointer) {
        return detail::NativeClosure<detail::function_signature_t<Func>>::template Create<detail::cpp_function<2>,
                                                                                          Func>(
            std::forward<Func>(func), vm, detail::cpp_function<2>::caller);
    } else {
        return detail::NativeClosure<detail::function_signature_t<Func>>::template Create<detail::cpp_function<1>,
                                                                                          Func>(
            std::forward<Func>(func), vm, detail::cpp_function<1>::caller);
    }
}

} // namespace detail

namespace detail {
// cast SQObject to NativeClosure
template <class Func> class GenericCast<NativeClosure<Func>(HSQOBJECT &)> {
  public:
    static NativeClosure<Func> cast(VM vm, HSQOBJECT &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast HSQOBJECT to " << typeid(NativeClosure<Func> &).name() << std::endl;
#endif
        return NativeClosure<Func>(_nativeclosure(obj), vm);
    }
};

template <class Func> class GenericCast<NativeClosure<Func>(HSQOBJECT &&)> {
  public:
    static NativeClosure<Func> cast(VM vm, HSQOBJECT &&obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast HSQOBJECT to " << typeid(NativeClosure<Func> &).name() << std::endl;
#endif
        return NativeClosure<Func>(_nativeclosure(obj), vm);
    }
};

// cast SQObjectPtr to NativeClosure
template <class Func> class GenericCast<NativeClosure<Func>(SQObjectPtr &)> {
  public:
    static NativeClosure<Func> cast(VM vm, SQObjectPtr &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(NativeClosure<Func> &).name() << std::endl;
#endif
        return NativeClosure<Func>(_nativeclosure(obj), vm);
    }
};

template <class Func> class GenericCast<NativeClosure<Func>(SQObjectPtr &&)> {
  public:
    static NativeClosure<Func> cast(VM vm, SQObjectPtr &&obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast SQObjectPtr to " << typeid(NativeClosure<Func> &).name() << std::endl;
#endif
        return NativeClosure<Func>(_nativeclosure(obj), vm);
    }
};

// cast NativeClosure to SQObjectPtr
template <class Func> class GenericCast<SQObjectPtr(std::shared_ptr<NativeClosure<Func>> &)> {
  public:
    static SQObjectPtr cast(VM vm, std::shared_ptr<NativeClosure<Func>> &obj) {
#ifdef TRACE_OBJECT_CAST
        std::cout << "[TRACING] cast " << typeid(NativeClosure<Func> &).name() << " to SQObjectPtr" << std::endl;
#endif
        return SQObjectPtr(obj->pNativeClosure());
    }
};
} // namespace detail
} // namespace sqbinding
