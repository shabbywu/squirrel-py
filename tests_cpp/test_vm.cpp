#include <iostream>
#include <string>
#include <type_traits>
#include <concepts>
#include <functional>
#include <sqbinding/detail/vm/vm.hpp>
#include <sqbinding/detail/common/type_traits.hpp>



template <class T>
void sig(T t) {
    std::cout << typeid(T).name() << std::endl;
    std::cout << "std::is_function_v=" << std::is_function_v<decltype(t)> << std::endl;
    std::cout << "std::is_member_pointer_v=" << std::is_member_pointer_v<decltype(t)> << std::endl;
    std::cout << "std::is_pointer_v=" << std::is_pointer_v<decltype(t)> << std::endl;
    std::cout << typeid(typename sqbinding::detail::function_traits<T>::type).name() << std::endl;
    std::cout << typeid(decltype(t)).name() << std::endl;
    std::cout << (int)sqbinding::detail::function_traits<T>::value << std::endl;;
}


void test () {
    std::cout << "Hello vanilla function pointer" << std::endl;
}

class A{
    public:
    void test () {
        std::cout << "Hello class method" << std::endl;
    }
};


void main(){
    auto func = std::function([](){
        std::cout << "Hello lambda" << std::endl;
    });
    {
        auto wrapper = sqbinding::detail::cpp_function(func);
        wrapper.operator()<void>();
    }
    {
        auto wrapper = sqbinding::detail::cpp_function([](){
            std::cout << "Hello lambda" << std::endl;
        });
        wrapper.operator()<void>();
    }
    {
        auto wrapper = sqbinding::detail::cpp_function(&test);
        wrapper.operator()<void>();
    }
    {
        auto wrapper = sqbinding::detail::cpp_function(&A::test);
        A a;
        wrapper.operator()<void, A*>(&a);
    }

    // sqbinding::detail::caller<decltype(func)>::call(&wrapper, &wrapper);
    // wrapper();
    // auto vm = sqbinding::detail::GenericVM();
    // cpp_function{test};
    // auto lambda = [](int x) -> std::string {
    //     return std::string("Hello World\n");
    // };
    // cpp_function{lambda};
    // auto class_test = &Class::test;
    // cpp_function{class_test};
    // auto error = cpp_function{1};
    std::cin.get();
}
