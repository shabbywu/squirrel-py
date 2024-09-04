#include <concepts>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include "sqbinding/detail/common/cpp_function.hpp"
#include <sqbinding/detail/common/cast_object.hpp>
#include <sqbinding/detail/common/type_traits.hpp>
#include <sqbinding/detail/types/sqclass.hpp>
#include <sqbinding/detail/types/sqfunction.hpp>
#include <sqbinding/detail/vm/vm.hpp>

using namespace sqbinding;

template <class T> void sig(T t) {
    std::cout << typeid(T).name() << std::endl;
    std::cout << "std::is_function_v=" << std::is_function_v<decltype(t)> << std::endl;
    std::cout << "std::is_member_pointer_v=" << std::is_member_pointer_v<decltype(t)> << std::endl;
    std::cout << "std::is_pointer_v=" << std::is_pointer_v<decltype(t)> << std::endl;
    std::cout << typeid(decltype(t)).name() << std::endl;
    std::cout << typeid(detail::function_signature_t<T>).name() << std::endl;
    std::cout << typeid(detail::function_traits<T>::type).name() << std::endl;
    std::cout << (int)(detail::function_traits<T>::value) << std::endl;
}

void vanillaFuncitonPointer(int i) {
    std::cout << "i: " << i << std::endl;
    std::cout << "Hello vanilla function pointer" << std::endl;
}

class A {
  public:
    int field = 1;

  public:
    static void static_method() {};
    void nonconst_method() {
        std::cout << "++field: " << ++field << std::endl;
    };
    void const_method() const {
        std::cout << "field: " << field << std::endl;
    };
};

void test_function_signature() {
    std::cout << "[*] vanillaFuncitonPointer" << std::endl;
    sig(&vanillaFuncitonPointer);
    {
        auto ref = &vanillaFuncitonPointer;
        std::cout << "[*] vanillaFuncitonPointer&" << std::endl;
        sig(ref);
    }
    std::cout << "[*] lambda(float)" << std::endl;
    sig([](float i) {});
    {
        auto ref = [](float i) {};
        std::cout << "[*] lambda(float)&" << std::endl;
        sig(ref);
    }
    std::cout << "[*] A::static_method" << std::endl;
    sig(&A::static_method);
    std::cout << "[*] A::nonconst_method" << std::endl;
    sig(&A::nonconst_method);
    std::cout << "[*] A::const_method" << std::endl;
    sig(&A::const_method);
    std::cout << "=========" << std::endl;
}

void test_cast_function_to_cpp_function() {
    // testcase for cast function to cpp_function
    {
        auto wrapper = detail::cpp_function<1>(std::function([]() { std::cout << "Hello lambda" << std::endl; }));
        wrapper.operator()<void>();
    }
    {
        auto wrapper = detail::cpp_function<1>([]() { std::cout << "Hello lambda" << std::endl; });
        wrapper.operator()<void>();
    }
    {
        auto wrapper = detail::cpp_function<1>(&vanillaFuncitonPointer);
        wrapper.operator()<void>(1);
    }

    {
        std::cout << "[*] calling A::static_method" << std::endl;
        A a;
        auto wrapper = detail::cpp_function<1>(&A::static_method);
        wrapper.operator()<void>();
    }

    {
        std::cout << "[*] calling A::nonconst_method" << std::endl;
        A a;
        auto wrapper = detail::cpp_function<2>(&A::nonconst_method);
        wrapper.operator()<void>(&a);
    }

    {
        std::cout << "[*] calling A::const_method" << std::endl;
        A a;
        auto wrapper = detail::cpp_function<2>(&A::const_method);
        wrapper.operator()<void>(&a);
    }
}

void test_call_non_class_function_in_vm() {
    auto vm = detail::GenericVM();

    int global_args = 1;
    // step: bind cpp function to squirrel
    {
        using namespace detail;
        vm.bindFunc("static_func", &vanillaFuncitonPointer);
        vm.bindFunc("lambda_func", [](int i) -> int {
            std::cout << "call from sq: " << i << std::endl;
            return 2;
        });
        vm.bindFunc("capture_func", [&global_args](int i) {
            global_args += i;
            std::cout << "global_args + i =" << global_args + i << std::endl;
        });
    }

    // step: call cpp function in squirrel
    try {
        vm.ExecuteString<void>("static_func(10086); return 1;");
        auto ret = vm.ExecuteString<int>("lambda_func(10086); return 2;");
        std::cout << "retuen value: " << ret << std::endl;
        vm.ExecuteString("print(capture_func + \"\\n\"); capture_func(1); capture_func(2);");
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}

void test_overload_func() {
    auto vm = detail::GenericVM();
    int global_args = 1;
    // step: bind cpp function to squirrel
    {
        using namespace detail;
        vm.bindFunc("func", []() { return 0; });
        vm.bindFunc("func", [](int i) { return i; });
        vm.bindFunc("func", [](int i, int j) { return i + j; });
    }

    try {
        vm.ExecuteString("print(\"func(): \" + func() + \"\\n\");");
        vm.ExecuteString("print(\"func(1): \" + func(1) + \"\\n\");");
        vm.ExecuteString("print(\"func(1, 2): \" + func(1, 2) + \"\\n\");");
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}

void main() {
    test_function_signature();
    std::cout << "============" << std::endl;
    test_cast_function_to_cpp_function();
    std::cout << "============" << std::endl;
    test_call_non_class_function_in_vm();
    std::cout << "============" << std::endl;
    test_overload_func();
    std::cout << "============" << std::endl;
    // Note：class_function test in test_cpp_class.cpp
    std::cin.get();
}
