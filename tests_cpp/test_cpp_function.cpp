#include <iostream>
#include <string>
#include <type_traits>
#include <concepts>
#include <functional>
#include <sqbinding/detail/vm/vm.hpp>
#include <sqbinding/detail/common/type_traits.hpp>
#include <sqbinding/detail/types/sqfunction.hpp>



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



class A{
    int i = 1;
    public:
        void nonconst_method () {
            i++;
            std::cout << "Hello nonconst class method: " << i << std::endl;
        }

        void const_method() const {
            std::cout << "Hello const class method: " << i << std::endl;
        }
};


void main(){
    // {
    //     auto wrapper = sqbinding::detail::cpp_function(std::function([](){
    //         std::cout << "Hello lambda" << std::endl;
    //     }));
    //     wrapper.operator()<void>();
    // }
    // {
    //     auto wrapper = sqbinding::detail::cpp_function([](){
    //         std::cout << "Hello lambda" << std::endl;
    //     });
    //     wrapper.operator()<void>();
    // }
    // {
    //     auto wrapper = sqbinding::detail::cpp_function(&test);
    //     wrapper.operator()<void>(1);
    // }

    // {
    //     auto seek = [](){
    //         using Type = typename decltype(&A::nonconst_method);
    //         std::cout << "Seeking" << std::endl;
    //         std::cout << typeid(Type).name() << std::endl;
    //         std::cout << typeid(sqbinding::detail::function_signature_t<Type>).name() << std::endl;
    //     };
    //     seek();
    //     auto wrapper = sqbinding::detail::cpp_function(&A::nonconst_method);
    // }

    // {
    //     sig(&A::const_method);
    //     auto wrapper = sqbinding::detail::cpp_function(&A::const_method);
    //     A a;
    //     wrapper.operator()<void, A*>(&a);
    // }
    int global_args = 1;
    A a;
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
        vm.bindFunc("nonconst_method", &A::nonconst_method);
        vm.getroottable()->set(std::string("a"), &a);
        A* a = vm.getroottable()->get<std::string, A*>(std::string("a"));

    }

    try
    {
        vm.ExecuteString("a.nonconst_method <- nonconst_method");
        vm.ExecuteString<void>("static_func(10086); return 1;");
        auto ret = vm.ExecuteString<int>("lambda_func(10086); return 2;");
        std::cout <<"retuen value: " << ret << std::endl;
        vm.ExecuteString("print(capture_func); capture_func(1); capture_func(2);");
        // vm.ExecuteString("nonconst_method()");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
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
