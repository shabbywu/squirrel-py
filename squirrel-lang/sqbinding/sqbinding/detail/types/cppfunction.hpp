#pragma once
#include <functional>
#include <memory>
#include "sqvm.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/detail/common/type_traits.hpp"

namespace sqbinding {
    namespace detail {
        class cpp_function {
            private:
                enum class FunctorOptions {
                    _false,
                    _true,
                };
            public:
                struct Holder {
                    /// Storage for the wrapped function pointer and captured data, if any
                    void *data[3] = {};

                    /// Pointer to custom destructor for 'data' (if needed)
                    void (*free_data)(Holder *ptr) = nullptr;

                    /// Pointer to squirrel caller
                    std::function<SQInteger(HSQUIRRELVM)> caller = nullptr;

                    bool functor = false;

                    ~Holder(){
                        if (free_data != nullptr) {
                            free_data(this);
                        }
                    }
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                cpp_function(std::nullptr_t) {}

                /// Construct a cpp_function from a vanilla function pointer
                template <typename Return, typename... Args>
                // NOLINTNEXTLINE(google-explicit-constructor)
                cpp_function(Return (*func)(Args...)) {
                    initialize<false>(func, func);
                }

                /// Construct a cpp_function from a class method (non-const, no ref-qualifier)
                template <typename Return, typename Class, typename... Args>
                // NOLINTNEXTLINE(google-explicit-constructor)
                cpp_function(Return (Class::*f)(Args...)) {
                    initialize<true>(std::function([f](Class *c, Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
                        (Return(*)(Class*, Args...)) nullptr
                    );
                }

                /// Construct a cpp_function from a class method (non-const, lvalue ref-qualifier)
                /// A copy of the overload for non-const functions without explicit ref-qualifier
                /// but with an added `&`.
                template <typename Return, typename Class, typename... Args>
                // NOLINTNEXTLINE(google-explicit-constructor)
                cpp_function(Return (Class::*f)(Args...) &) {
                    initialize<true>(std::function([f](Class *c, Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
                        (Return(*)(Class *, Args...)) nullptr
                    );
                }

                /// Construct a cpp_function from a class method (const, no ref-qualifier)
                template <typename Return, typename Class, typename... Args>
                // NOLINTNEXTLINE(google-explicit-constructor)
                cpp_function(Return (Class::*f)(Args...) const) {
                    initialize<true>(std::function([f](const Class *c,
                                Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
                                (Return(*)(const Class *, Args...)) nullptr
                    );
                }

                /// Construct a cpp_function from a class method (const, lvalue ref-qualifier)
                /// A copy of the overload for const functions without explicit ref-qualifier
                /// but with an added `&`.
                template <typename Return, typename Class, typename... Args>
                // NOLINTNEXTLINE(google-explicit-constructor)
                cpp_function(Return (Class::*f)(Args...) const &) {
                    initialize<true>(std::function([f](const Class *c,
                                Args... args) -> Return { return (c->*f)(std::forward<Args>(args)...); }),
                                (Return(*)(const Class *, Args...)) nullptr
                    );
                }

                /// Construct a cpp_function from a lambda function (possibly with internal state)
                template <class Func, typename = std::enable_if_t<function_traits<std::remove_reference_t<Func>>::value == CppFuntionType::LambdaLike>>
                cpp_function(Func&& func) {
                    // initialize(std::forward<Func>(func), (typename function_traits<Func>::type*)nullptr);
                    initialize<true>(std::function(std::forward<Func>(func)), (function_signature_t<Func>*)nullptr);
                }

            public:
                /// Special internal constructor for functors, lambda functions, etc.
                template <bool functor, class Func, typename Return, typename... Args, std::enable_if_t<functor, bool> = true>
                void initialize(Func&& f, Return (*signature)(Args...)) {
                    struct capture {
                        std::remove_reference_t<Func> f;
                    };
                    holder = std::make_shared<Holder>();
                    holder->functor = true;

                    /* Store the capture object directly in the function record if there is enough space */
                    if (sizeof(capture) <= sizeof(holder->data)) {
                        new ((capture *) &holder->data) capture{std::forward<Func>(f)};
                    } else {
                        holder->data[0] = new capture{std::forward<Func>(f)};
                        holder->free_data = [](Holder *r) { delete ((capture *) r->data[0]); };
                    }

                    holder->caller = std::function([f, this](HSQUIRRELVM vm) -> SQInteger {
                        cpp_function* func;
                        sq_getuserdata(vm, -1, (void**)&func, NULL);
                        int nparams = sq_gettop(vm) - 2;

                        // 索引从 1 开始, 且位置 1 是 this(env)
                        // 参数从索引 2 开始
                        auto vm_ = detail::VM(vm);
                        auto functor = detail::load_args<2, std::function<Return(Args...)>>::load(f, vm_);
                        return 1;
                    });
                }

                template <bool functor, class Func, typename Return, typename... Args, std::enable_if_t<!functor, bool> = true>
                void initialize(Func&& f, Return (*signature)(Args...)) {
                    struct capture {
                        std::remove_reference_t<Func> f;
                    };
                    holder = std::make_shared<Holder>();
                    holder->functor = false;

                    /* Store the capture object directly in the function record if there is enough space */
                    if (sizeof(capture) <= sizeof(holder->data)) {
                        new ((capture *) &holder->data) capture{std::forward<Func>(f)};
                    } else {
                        holder->data[0] = new capture{std::forward<Func>(f)};
                        holder->free_data = [](Holder *r) { delete ((capture *) r->data[0]); };
                    }

                    holder->caller = build_caller<functor>(f);
                }

            public:
                template <bool functor, class Func, typename Return, typename... Args, std::enable_if_t<!functor, bool> = true>
                std::function<SQInteger(HSQUIRRELVM)> build_caller(Func&& f) {
                    return std::function([f, this](HSQUIRRELVM vm) -> SQInteger{
                        // 索引从 1 开始, 且位置 1 是 this(env)
                        // 参数从索引 2 开始
                        auto vm_ = detail::VM(vm);
                        auto load_func = detail::load_args<2, Return(Args...)>::load(f, vm_);
                        Return v = load_func();
                        generic_stack_push(vm, v);
                        return 1;
                    });
                }

                template <bool functor, class Func, typename... Args, std::enable_if_t<!functor, bool> = true>
                std::function<SQInteger(HSQUIRRELVM)> build_caller(Func&& f) {
                    return std::function([f, this](HSQUIRRELVM vm) -> SQInteger{
                        // 索引从 1 开始, 且位置 1 是 this(env)
                        // 参数从索引 2 开始
                        auto vm_ = detail::VM(vm);
                        auto load_func = detail::load_args<2, void(Args...)>::load(f, vm_);
                        load_func();
                        return 0;
                    });
                }

            public:
                template <typename Return, typename... Args>
                Return operator()(Args... args) {
                    if (holder->functor) {
                        auto func = (std::function<Return(Args...)>*) holder->data[0];
                        return (*func)(args...);
                    } else {
                        auto func = (Return(*)(Args...)) holder->data[0];
                        return (*func)(args...);
                    }
                }
        };
    }
}
