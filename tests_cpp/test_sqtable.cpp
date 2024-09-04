#include <concepts>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/cast.hpp>
#include <sqbinding/detail/types.hpp>
#include <sqbinding/detail/vm/vm.hpp>

using namespace sqbinding;

void test_get_and_call_sqclosure(detail::GenericVM vm) {
    auto context = vm.ExecuteString<detail::Table>(R"(
        local rt = getroottable();
        local context = {
            value = 1
            function staticFunc () {
                print("call staticfunc\n");
            }
            function increseValue () {
                value += 1;
                print("call contextFunc: " + value + "\n");
            }
        };
        return context;
    )");
    auto anotherContext = vm.ExecuteString<detail::Table>(R"(
        return {
            value = 10086
        }
    )");

    std::cout << "=========" << std::endl;
    try {
        context.get<std::string, detail::Closure<void(void)>>("staticFunc")();
        context.get<std::string, detail::Closure<void(void)>>("increseValue")();
        std::cout << context.get<std::string, int>("value") << std::endl;

        auto increseValue = context.get<std::string, detail::Closure<void(void)>>("increseValue");
        auto bound = increseValue.get<std::string, detail::NativeClosure<decltype(increseValue)(SQObjectPtr)>>(
            "bindenv")(anotherContext.holder->GetSQObjectPtr());
        bound();
    }
    catch (const std::exception &e) {
        std::cerr << "Exception:: " << e.what() << '\n';
    }
}

void test_get_and_call_sqnativeclosure(detail::GenericVM vm) {
    std::cout << "=========" << std::endl;
    try {
        auto rt = vm.getroottable();
        rt->get<std::string, detail::NativeClosure<void(std::string)>>("print")("test\n");
    }
    catch (const std::exception &e) {
        std::cerr << "Exception:: " << e.what() << '\n';
    }
}

void test_setter(detail::GenericVM vm) {
    auto rt = vm.getroottable();
    rt->set(std::string(""), std::string(""));
    rt->bindFunc("test", []() {});
}

void main() {
    {
        auto vm = detail::GenericVM();
        test_get_and_call_sqclosure(vm);
        test_get_and_call_sqnativeclosure(vm);
        test_setter(vm);
    }
    std::cin.get();
}
