#pragma once
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include <map>
#include <functional>
#include <memory>

namespace sqbinding {
namespace detail {
class generic_function {
  protected:
    using Caller = SQInteger(VM);
    using StaticCaller = SQInteger(HSQUIRRELVM);

  public:
    virtual std::function<Caller> get_caller_impl() = 0;
    virtual StaticCaller *get_static_caller() = 0;
    virtual int get_nargs() = 0;
};

template <int paramsbase> class cpp_function : public generic_function {
  protected:
    struct Holder {
        /// Storage for the wrapped function pointer and captured data, if any
        void *data[3] = {};

        /// Pointer to custom destructor for 'data' (if needed)
        void (*free_data)(Holder *ptr) = nullptr;

        /// Pointer to squirrel caller
        std::function<Caller> caller = nullptr;

        bool functor = false;

        ~Holder() {
#ifdef TRACE_CONTAINER_GC
            std::cout << "GC::Release " << typeid(Holder).name() << std::endl;
#endif
            if (free_data != nullptr) {
                free_data(this);
            }
        }
    };

  protected:
    std::shared_ptr<Holder> holder;
    int nargs;

  public:
    virtual std::function<Caller> get_caller_impl() {
        return holder->caller;
    }
    virtual int get_nargs() {
        return nargs;
    }
    virtual StaticCaller *get_static_caller() {
        return &cpp_function::caller;
    }

  public:
    cpp_function(std::nullptr_t) {
    }

    /// Construct a cpp_function from a vanilla function pointer
    template <typename Return, typename... Args>
    // NOLINTNEXTLINE(google-explicit-constructor)
    cpp_function(Return (*func)(Args...)) {
        initialize<false>(func, func);
        nargs = sizeof...(Args);
    }

    /// Construct a cpp_function from a class method (non-const, no ref-qualifier)
    template <typename Return, typename Class, typename... Args>
    // NOLINTNEXTLINE(google-explicit-constructor)
    cpp_function(Return (Class::*f)(Args...)) {
        initialize<true>(
            std::function([f](Class *c, Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
            (Return(*)(Class *, Args...)) nullptr);
        nargs = sizeof...(Args);
    }

    /// Construct a cpp_function from a class method (non-const, lvalue ref-qualifier)
    /// A copy of the overload for non-const functions without explicit ref-qualifier
    /// but with an added `&`.
    template <typename Return, typename Class, typename... Args>
    // NOLINTNEXTLINE(google-explicit-constructor)
    cpp_function(Return (Class::*f)(Args...) &) {
        initialize<true>(
            std::function([f](Class *c, Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
            (Return(*)(Class *, Args...)) nullptr);
        nargs = sizeof...(Args);
    }

    /// Construct a cpp_function from a class method (const, no ref-qualifier)
    template <typename Return, typename Class, typename... Args>
    // NOLINTNEXTLINE(google-explicit-constructor)
    cpp_function(Return (Class::*f)(Args...) const) {
        initialize<true>(
            std::function([f](const Class *c, Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
            (Return(*)(const Class *, Args...)) nullptr);
        nargs = sizeof...(Args);
    }

    /// Construct a cpp_function from a class method (const, lvalue ref-qualifier)
    /// A copy of the overload for const functions without explicit ref-qualifier
    /// but with an added `&`.
    template <typename Return, typename Class, typename... Args>
    // NOLINTNEXTLINE(google-explicit-constructor)
    cpp_function(Return (Class::*f)(Args...) const &) {
        initialize<true>(
            std::function([f](const Class *c, Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
            (Return(*)(const Class *, Args...)) nullptr);
        nargs = sizeof...(Args);
    }

    /// Construct a cpp_function from a lambda function (possibly with internal state)
    template <class Func, typename = std::enable_if_t<function_traits<std::remove_reference_t<Func>>::value ==
                                                      CppFuntionType::LambdaLike>>
    cpp_function(Func &&func) {
        // initialize(std::forward<Func>(func), (typename function_traits<Func>::type*)nullptr);
        initialize<true>(std::function(std::forward<Func>(func)), (function_signature_t<Func> *)nullptr);
        nargs = function_traits<std::remove_reference_t<Func>>::nargs;
    }

  private:
    /// Special internal constructor for functors, lambda functions, etc.
    template <bool functor, class Func, typename Return, typename... Args>
    void initialize(Func &&f, Return (*signature)(Args...)) {
        struct capture {
            std::remove_reference_t<Func> f;
        };
        holder = std::make_shared<Holder>();
        holder->functor = functor;

        /* Store the capture object directly in the function record if there is enough space */
        if (sizeof(capture) <= sizeof(holder->data)) {
            new ((capture *)&holder->data) capture{std::forward<Func>(f)};
        } else {
            holder->data[0] = new capture{std::forward<Func>(f)};
            holder->free_data = [](Holder *r) { delete ((capture *)r->data[0]); };
        }

        holder->caller = build_caller<Func, Return, Args...>(std::forward<Func>(f));
    }

  private:
    template <class Func, typename Return, typename... Args>
    std::function<Caller> build_caller(
        Func &&f, Return (*signature)(Args...) = nullptr,
        typename std::enable_if_t<std::is_void_v<Return>, Return> * = nullptr) {
        return std::function([f, this](detail::VM vm) -> SQInteger {
            // 索引从 1 开始, 且位置 1 是 this(env)
            // 参数从索引 2 开始
            auto args = detail::load_args<paramsbase, std::tuple<Args...>>::load(vm);
            try {
                if (holder->functor) {
                    auto func = (std::function<Return(Args...)> *)holder->data[0];
                    std::apply(*func, args);
                } else {
                    auto func = (Return(*)(Args...))holder->data[0];
                    std::apply(*func, args);
                }
            }
            catch (const std::exception &e) {
                return sq_throwerror(*vm, e.what());
            }
            return 0;
        });
    }

    template <class Func, typename Return, typename... Args>
    std::function<Caller> build_caller(
        Func &&f, Return (*signature)(Args...) = nullptr,
        typename std::enable_if_t<!std::is_void_v<Return>, Return> * = nullptr) {
        return std::function([f, this](detail::VM vm) -> SQInteger {
            // 索引从 1 开始, 且位置 1 是 this(env)
            // 参数从索引 2 开始
            auto args = detail::load_args<paramsbase, std::tuple<Args...>>::load(vm);
            try {
                if (holder->functor) {
                    auto func = (std::function<Return(Args...)> *)holder->data[0];
                    Return v = std::apply(*func, args);
                    generic_stack_push<Return>(vm, std::forward<Return>(v));
                } else {
                    auto func = (Return(*)(Args...))holder->data[0];
                    Return v = std::apply(*func, args);
                    generic_stack_push<Return>(vm, std::forward<Return>(v));
                }
            }
            catch (const std::exception &e) {
                return sq_throwerror(*vm, e.what());
            }

            return 1;
        });
    }

  public:
    template <typename Return, typename... Args> Return operator()(Args... args) {
        if (holder->functor) {
            auto func = (std::function<Return(Args...)> *)holder->data[0];
            return (*func)(args...);
        } else {
            auto func = (Return(*)(Args...))holder->data[0];
            return (*func)(args...);
        }
    }

  public:
    static SQInteger caller(HSQUIRRELVM vm) {
        using Holder = detail::StackObjectHolder<cpp_function>;
        Holder *ud_holder;
        sq_getuserdata(vm, -1, (void **)&ud_holder, NULL);
        return ud_holder->GetInstance().holder->caller(ud_holder->vm);
    }
};

template <int stackbase, typename Func> static std::shared_ptr<generic_function> to_cpp_function(Func &&func) {
    return std::make_shared<cpp_function<stackbase>>(std::forward<Func>(func));
}

template <typename Func> static std::shared_ptr<generic_function> to_cpp_function(Func &&func) {
    if (detail::function_traits<Func>::value == detail::CppFuntionType::LambdaLike ||
        detail::function_traits<Func>::value == detail::CppFuntionType::VanillaFunctionPointer) {
        return to_cpp_function<2, Func>(std::forward<Func>(func));
    } else {
        return to_cpp_function<1, Func>(std::forward<Func>(func));
    }
}

} // namespace detail
namespace detail {
class overloaded_function : public generic_function {
  private:
    std::map<SQInteger, std::shared_ptr<generic_function>> callers;

  public:
    template <typename Func>
    void add_caller(Func&& func) {
        // TODO: rewrite with emplace
        auto caller = to_cpp_function(std::forward<Func>(func));
        callers[caller->get_nargs()] = caller;
    }

    void add_caller(std::shared_ptr<generic_function> caller) {
        callers[caller->get_nargs()] = caller;
    }

  public:
    virtual std::function<Caller> get_caller_impl() {
        return std::function([this](detail::VM vm) { return call(vm); });
    }
    virtual StaticCaller *get_static_caller() {
        return &overloaded_function::caller;
    }
    virtual int get_nargs() {
        return -1;
    }

  public:
    SQInteger call(detail::VM vm) {
        // FIXME: 2 硬编码会导致不支持类函数重载
        SQInteger nargs = sq_gettop(*vm) - 2;
        return callers[nargs]->get_caller_impl()(vm);
    }

  public:
    static SQInteger caller(HSQUIRRELVM vm) {
        using Holder = detail::StackObjectHolder<overloaded_function>;
        Holder *ud_holder;
        sq_getuserdata(vm, -1, (void **)&ud_holder, NULL);
        return ud_holder->GetInstance().call(ud_holder->vm);
    }
};

} // namespace detail
} // namespace sqbinding
