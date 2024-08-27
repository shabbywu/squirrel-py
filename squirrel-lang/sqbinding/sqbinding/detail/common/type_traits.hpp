#pragma once
#include <cstddef>
#include <type_traits>

namespace sqbinding {
namespace detail {
template <class Func> struct functor_traits;

// function from a functor method (non-const, no ref-qualifier)
template <typename Return, typename Class, typename... Args> struct functor_traits<Return (Class::*)(Args...)> {
    typedef Return type(Args...);
    static constexpr int nargs = sizeof...(Args);
};

// function from a functor method (non-const, lvalue ref-qualifier)
template <typename Return, typename Class, typename... Args> struct functor_traits<Return (Class::*)(Args...) &> {
    typedef Return type(Args...) &;
    static constexpr int nargs = sizeof...(Args);
};

// function from a functor method (const, no ref-qualifier)
template <typename Return, typename Class, typename... Args> struct functor_traits<Return (Class::*)(Args...) const> {
    typedef Return type(Args...);
    static constexpr int nargs = sizeof...(Args);
};

// function from a functor method (const, lvalue ref-qualifier)
template <typename Return, typename Class, typename... Args> struct functor_traits<Return (Class::*)(Args...) const &> {
    typedef Return type(Args...) &;
    static constexpr int nargs = sizeof...(Args);
};

enum class CppFuntionType {
    LambdaLike = 0b1,
    VanillaFunctionPointer = 0b10,
    ClassMethodNonConstNoRef = 0b100,
    ClassMethodNonConstRef = 0b1000,
    ClassMethodConstNoRef = 0b10000,
    ClassMethodConstRef = 0b100000,
};

template <typename Func>
struct function_traits : public functor_traits<decltype(&std::remove_reference_t<Func>::operator())> {
    static constexpr CppFuntionType value = CppFuntionType::LambdaLike;
};

// function from vanilla function pointer
template <typename Return, typename... Args> struct function_traits<Return (*)(Args...)> {
    typedef Return type(Args...);
    static constexpr CppFuntionType value = CppFuntionType::VanillaFunctionPointer;
};

// function from a class method (non-const, no ref-qualifier)
template <typename Return, typename Class, typename... Args> struct function_traits<Return (Class::*)(Args...)> {
    typedef Return type(Class *, Args...);
    static constexpr CppFuntionType value = CppFuntionType::ClassMethodNonConstNoRef;
};

// function from a class method (non-const, lvalue ref-qualifier)
template <typename Return, typename Class, typename... Args> struct function_traits<Return (Class::*)(Args...) &> {
    typedef Return type(Class *, Args...) &;
    static constexpr CppFuntionType value = CppFuntionType::ClassMethodNonConstRef;
};

// function from  a class method (const, no ref-qualifier)
template <typename Return, typename Class, typename... Args> struct function_traits<Return (Class::*)(Args...) const> {
    typedef Return type(const Class *, Args...) const;
    static constexpr CppFuntionType value = CppFuntionType::ClassMethodConstNoRef;
};

// function from  a class method (const, lvalue ref-qualifier)
template <typename Return, typename Class, typename... Args>
struct function_traits<Return (Class::*)(Args...) const &> {
    typedef Return type(const Class *, Args...) const &;
    static constexpr CppFuntionType value = CppFuntionType::ClassMethodConstNoRef;
    static constexpr int nargs = sizeof...(Args);
};

/// Strip the class from a method type
template <typename T> struct remove_class {};

template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> {
    using type = R(A...);
};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> {
    using type = R(A...);
};

// Extracts the function signature from a function, function pointer or lambda.
template <typename Function> using function_signature_t = typename function_traits<std::decay_t<Function>>::type;

} // namespace detail
} // namespace sqbinding
