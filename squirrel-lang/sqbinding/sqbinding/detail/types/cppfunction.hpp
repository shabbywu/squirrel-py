#pragma once
#include <functional>
#include <memory>
#include "sqvm.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/malloc.hpp"

namespace sqbinding {
    namespace detail {
        class cpp_function {
            public:
                struct Holder {
                    /// Storage for the wrapped function pointer and captured data, if any
                    void *data[3] = {};

                    /// Pointer to custom destructor for 'data' (if needed)
                    void (*free_data)(Holder *ptr) = nullptr;

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
                    initialize(std::function(func));
                }

                /// Construct a cpp_function from a class method (non-const, no ref-qualifier)
                template <typename Return, typename Class, typename... Arg>
                // NOLINTNEXTLINE(google-explicit-constructor)
                cpp_function(Return (Class::*f)(Arg...)) {
                    initialize(
                        std::function([f](Class *c, Arg... args) -> Return { return (c->*f)(std::forward<Arg>(args)...); })
                    );
                }

                /// Construct a cpp_function from a lambda function (possibly with internal state)
                template <class Func, typename = std::enable_if_t<function_traits<Func>::value == CppFuntionType::LambdaLike>>
                cpp_function(Func func) {
                    static_assert(function_traits<Func>::value == CppFuntionType::LambdaLike);
                    initialize(std::function(func));
                }

            public:
                /// Special internal constructor for functors, lambda functions, etc.
                template <class Func>
                void initialize(Func &&f) {
                    struct capture {
                        std::remove_reference_t<Func> f;
                    };
                    holder = std::make_shared<Holder>();
                    /* Store the capture object directly in the function record if there is enough space */
                    if (sizeof(capture) <= sizeof(holder->data)) {
                        new ((capture *) &holder->data) capture{std::forward<Func>(f)};
                    } else {
                        holder->data[0] = new capture{std::forward<Func>(f)};
                        holder->free_data = [](Holder *r) { delete ((capture *) r->data[0]); };
                    }
                }
            public:
                template <typename Return, typename... Args>
                Return operator()(Args... args) {
                    auto func = (std::function<Return(Args...)>*) holder->data[0];
                    return (*func)(args...);
                }
        };

        template <typename Func>
        struct caller;

        template <typename Return, typename... Args>
        struct caller<Return(Args...)> {
            static Return call(cpp_function* self, Args... args) {
                Return (*func)(Args...) = (Return(*)(Args...))self->holder->data[0];
                return func(args...);
            }
        };

        template <class T>
        class CppFunction;

        template <class Return, class...Args>
        class CppFunction<Return (Args...)> {
            public:
                struct Holder {
                    Holder(std::function<Return(Args...)>& func, detail::VM vm): func(func), vm(vm){};

                    detail::VM vm;
                    std::function<Return(Args...)> func;
                };
            public:
                CppFunction(std::function<Return(Args...)>& func, detail::VM vm): holder(std::make_shared<Holder>(func, vm)) {}
            public:
                std::shared_ptr<std::function<Return(Args...)>> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call
            public:
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                Return operator()(Args... args) {
                    VM& vm = holder->vm;
                    stack_guard stack_guard(vm);
                    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
                        call_setup(vm, holder->closure, pthis, args...);
                    } else {
                        call_setup(vm, holder->closure, (*vm)->_roottable, args...);
                    }
                    return call<Return>(vm, stack_guard.offset() - 1);
                }
            public:
                static SQUserData* Create(std::function<Return(Args...)>& func, detail::VM vm) {
                    // new userdata to store py::function
                    auto result = detail::make_stack_object<CppFunction, std::function<Return(Args...)>, detail::VM>(vm, func, vm);
                    auto pycontainer = result.first;
                    auto ud = result.second;
                    ud->SetDelegate(pycontainer->_delegate->pTable());
                    ud->_typetag = typeid(Return(Args...)).hash_code();
                    return ud;
                }

                static SQUserData* Create(Return(*func)(Args...) , detail::VM vm) {
                    // new userdata to store py::function
                    auto result = detail::make_stack_object<CppFunction, std::function<Return(Args...)>, detail::VM>(vm, func, vm);
                    auto pycontainer = result.first;
                    auto ud = result.second;
                    ud->SetDelegate(pycontainer->_delegate->pTable());
                    ud->_typetag = typeid(Return(Args...)).hash_code();
                    return ud;
                }
        };
    }
}
