#pragma once
#include <functional>
#include <memory>
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/types/sqvm.hpp"


namespace py = pybind11;


namespace sqbinding {
    namespace python {
        // 关于 paramsbase 取值
        // 索引编号从 1 开始, 索引 1 是 userdata(该函数), 索引 2 是 this(env), 参数从索引 3 开始
        // 需要 env 时, paramsbase 设成 2
        // 无需 env 时, paramsbase 设成 3
        template <int paramsbase>
        class dynamic_args_function {
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
                dynamic_args_function(std::nullptr_t) {}

                /// Construct a dynamic_args_function from a vanilla function pointer
                template <typename Return>
                // NOLINTNEXTLINE(google-explicit-constructor)
                dynamic_args_function(Return (*func)(py::list)) {
                    initialize<false>(func, func);
                }

                /// Construct a dynamic_args_function from a class method (non-const, no ref-qualifier)
                template <typename Return, typename Class>
                // NOLINTNEXTLINE(google-explicit-constructor)
                dynamic_args_function(Return (Class::*f)(py::list)) {
                    initialize<true>(std::function([f](Class *c, py::list args) -> Return { return (c->*f)(args); }),
                        (Return(*)(Class*, py::list)) nullptr
                    );
                }

                /// Construct a dynamic_args_function from a class method (non-const, lvalue ref-qualifier)
                /// A copy of the overload for non-const functions without explicit ref-qualifier
                /// but with an added `&`.
                template <typename Return, typename Class>
                // NOLINTNEXTLINE(google-explicit-constructor)
                dynamic_args_function(Return (Class::*f)(py::list) &) {
                    initialize<true>(std::function([f](Class *c, py::list args) -> Return { return (c->*f)(args); }),
                        (Return(*)(Class*, py::list)) nullptr
                    );
                }

                /// Construct a dynamic_args_function from a class method (const, no ref-qualifier)
                template <typename Return, typename Class>
                // NOLINTNEXTLINE(google-explicit-constructor)
                dynamic_args_function(Return (Class::*f)(py::list) const) {
                    initialize<true>(std::function([f](const Class *c,
                                py::list args) -> Return { return (c->*f)(args); }),
                                (Return(*)(const Class *, py::list)) nullptr
                    );
                }

                /// Construct a dynamic_args_function from a class method (const, lvalue ref-qualifier)
                /// A copy of the overload for const functions without explicit ref-qualifier
                /// but with an added `&`.
                template <typename Return, typename Class>
                // NOLINTNEXTLINE(google-explicit-constructor)
                dynamic_args_function(Return (Class::*f)(py::list) const &) {
                    initialize<true>(std::function([f](const Class *c,
                                py::list args) -> Return { return (c->*f)(args); }),
                                (Return(*)(const Class *, py::list)) nullptr
                    );
                }

                /// Construct a dynamic_args_function from a lambda function (possibly with internal state)
                template <class Func, typename = std::enable_if_t<detail::function_traits<std::remove_reference_t<Func>>::value == detail::CppFuntionType::LambdaLike>>
                dynamic_args_function(Func&& func) {
                    // initialize(std::forward<Func>(func), (typename function_traits<Func>::type*)nullptr);
                    initialize<true>(std::function(std::forward<Func>(func)), (detail::function_signature_t<Func>*)nullptr);
                }

            public:
                /// Special internal constructor for functors, lambda functions, etc.
                template <bool functor, class Func, typename Return>
                void initialize(Func&& f, Return (*signature)(py::list)) {
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

                    holder->caller = build_caller<std::remove_reference_t<Func>, Return>(std::forward<Func>(f));
                }

            public:
                template <class Func, typename Return>
                std::function<SQInteger(HSQUIRRELVM)> build_caller(Func&& f, typename std::enable_if_t<std::is_void_v<Return>, Return>* = nullptr) {
                    return std::function([f, this](HSQUIRRELVM vm) -> SQInteger{
                        auto vm_ = detail::VM(vm);
                        auto args = detail::load_args<paramsbase, py::list>::load(vm_);
                        this->operator()<void>(args);
                        return 0;
                    });
                }

                template <class Func, typename Return>
                std::function<SQInteger(HSQUIRRELVM)> build_caller(Func&& f, typename std::enable_if_t<!std::is_void_v<Return>, Return>* = nullptr) {
                    return std::function([f, this](HSQUIRRELVM vm) -> SQInteger{
                        auto vm_ = detail::VM(vm);
                        auto args = detail::load_args<paramsbase, py::list>::load(vm_);
                        Return v = this->operator()<Return>(args);
                        detail::generic_stack_push(vm, v);
                        return 1;
                    });
                }
            public:
                template <typename Return>
                Return operator()(py::list args) {
                    if (holder->functor) {
                        auto func = (std::function<Return(py::list)>*) holder->data[0];
                        return (*func)(args);
                    } else {
                        auto func = (Return(*)(py::list)) holder->data[0];
                        return (*func)(args);
                    }
                }

            static SQInteger caller(HSQUIRRELVM vm) {
                py::gil_scoped_acquire acquire;
                struct StackObjectHolder {
                    std::shared_ptr<dynamic_args_function<paramsbase>> instance;
                }* ud_holder;
                sq_getuserdata(vm, -1, (void**)&ud_holder, NULL);
                return ud_holder->instance->holder->caller(vm);
            }
        };
    }
}
