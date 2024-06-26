#pragma once
#include <functional>
#include <memory>
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/types/sqvm.hpp"


namespace sqbinding {
    namespace detail {
        template <int paramsbase>
        class cpp_function {
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
                       #ifdef TRACE_CONTAINER_GC
                            std::cout << "GC::Release " << typeid(Holder).name() << std::endl;
                        #endif
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
                template <bool functor, class Func, typename Return, typename... Args>
                void initialize(Func&& f, Return (*signature)(Args...)) {
                    struct capture {
                        std::remove_reference_t<Func> f;
                    };
                    holder = std::make_shared<Holder>();
                    holder->functor = functor;

                    /* Store the capture object directly in the function record if there is enough space */
                    if (sizeof(capture) <= sizeof(holder->data)) {
                        new ((capture *) &holder->data) capture{std::forward<Func>(f)};
                    } else {
                        holder->data[0] = new capture{std::forward<Func>(f)};
                        holder->free_data = [](Holder *r) { delete ((capture *) r->data[0]); };
                    }

                    holder->caller = build_caller<Func, Return, Args...>(std::forward<Func>(f));
                }

            public:
                template <class Func, typename Return, typename... Args>
                std::function<SQInteger(HSQUIRRELVM)> build_caller(Func&& f, Return (*signature)(Args...) = nullptr, typename std::enable_if_t<std::is_void_v<Return>, Return>* = nullptr) {
                    return std::function([f, this](HSQUIRRELVM vm) -> SQInteger{
                        // 索引从 1 开始, 且位置 1 是 this(env)
                        // 参数从索引 2 开始
                        auto vm_ = detail::VM(vm);
                        auto args = detail::load_args<paramsbase, std::tuple<Args...>>::load(vm_);
                        if (holder->functor) {
                            auto func = (std::function<Return(Args...)>*) holder->data[0];
                            std::apply(*func, args);
                        } else {
                            auto func = (Return(*)(Args...)) holder->data[0];
                            std::apply(*func, args);
                        }
                        return 0;
                    });
                }

                template <class Func, typename Return, typename... Args>
                std::function<SQInteger(HSQUIRRELVM)> build_caller(Func&& f, Return (*signature)(Args...) = nullptr, typename std::enable_if_t<!std::is_void_v<Return>, Return>* = nullptr) {
                    return std::function([f, this](HSQUIRRELVM vm) -> SQInteger{
                        // 索引从 1 开始, 且位置 1 是 this(env)
                        // 参数从索引 2 开始
                        auto vm_ = detail::VM(vm);
                        auto args = detail::load_args<paramsbase, std::tuple<Args...>>::load(vm_);
                        if (holder->functor) {
                            auto func = (std::function<Return(Args...)>*) holder->data[0];
                            Return v = std::apply(*func, args);
                            generic_stack_push(vm, v);
                        } else {
                            auto func = (Return(*)(Args...)) holder->data[0];
                            Return v = std::apply(*func, args);
                            generic_stack_push(vm, v);
                        }
                        return 1;
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
            public:
            static SQInteger caller(HSQUIRRELVM vm) {
                using Holder = detail::StackObjectHolder<cpp_function>;
                Holder* ud_holder;
                sq_getuserdata(vm, -1, (void**)&ud_holder, NULL);
                return ud_holder->GetInstance().holder->caller(vm);
            }
        };
    }
}
