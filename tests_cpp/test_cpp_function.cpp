#include <iostream>
#include <string>
#include <type_traits>
#include <concepts>
#include <functional>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/vm/vm.hpp>
#include <sqbinding/detail/common/type_traits.hpp>
#include <sqbinding/detail/types/sqfunction.hpp>
#include <sqbinding/detail/types/sqclass.hpp>
#include "sqbinding/detail/common/cpp_function.hpp"
#include <sqbinding/detail/common/cast_object.hpp>


template <class T>
void sig(T t) {
    std::cout << typeid(T).name() << std::endl;
    std::cout << "std::is_function_v=" << std::is_function_v<decltype(t)> << std::endl;
    std::cout << "std::is_member_pointer_v=" << std::is_member_pointer_v<decltype(t)> << std::endl;
    std::cout << "std::is_pointer_v=" << std::is_pointer_v<decltype(t)> << std::endl;
    std::cout << typeid(decltype(t)).name() << std::endl;
    std::cout << typeid(sqbinding::detail::function_signature_t<T>).name() << std::endl;
    std::cout << typeid(sqbinding::detail::function_traits<T>::type).name() << std::endl;
    std::cout << (int)(sqbinding::detail::function_traits<T>::value) << std::endl;
}


void test (int i) {
    std::cout << "i: " << i << std::endl;
    std::cout << "Hello vanilla function pointer" << std::endl;
}


void main(){
    // testcase for cast function to cpp_function
    {
        auto wrapper = sqbinding::detail::cpp_function<1>(std::function([](){
            std::cout << "Hello lambda" << std::endl;
        }));
        wrapper.operator()<void>();
    }
    {
        auto wrapper = sqbinding::detail::cpp_function<1>([](){
            std::cout << "Hello lambda" << std::endl;
        });
        wrapper.operator()<void>();
    }
    {
        auto wrapper = sqbinding::detail::cpp_function<1>(&test);
        wrapper.operator()<void>(1);
    }

    // testcase for bind cpp function to squirrel
    int global_args = 1;
    auto vm = sqbinding::detail::GenericVM();
    {
        using namespace sqbinding::detail;
        vm.bindFunc("static_func", &test);
        vm.bindFunc("lambda_func", [](int i) -> int {
            std::cout << "call from sq: " << i << std::endl;
            return 2;
        });
        vm.bindFunc("capture_func", [&global_args](int i) {
            global_args +=i;
            std::cout << "global_args + i =" << global_args + i << std::endl;
        });
    }

    // testcase for call cpp function from squirrel
    try
    {
        vm.ExecuteString<void>("static_func(10086); return 1;");
        auto ret = vm.ExecuteString<int>("lambda_func(10086); return 2;");
        std::cout <<"retuen value: " << ret << std::endl;
        vm.ExecuteString("print(capture_func + \"\\n\"); capture_func(1); capture_func(2);");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    std::cin.get();
}
