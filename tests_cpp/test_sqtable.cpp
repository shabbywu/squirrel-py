#include <iostream>
#include <string>
#include <type_traits>
#include <concepts>
#include <functional>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/vm/vm.hpp>
#include <sqbinding/detail/types/sqfunction.hpp>
#include <sqbinding/detail/common/cast_object.hpp>

using namespace sqbinding;

void main(){
    auto vm = detail::GenericVM();
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
    try
    {
        context.get<std::string, detail::Closure<void(void)>>("staticFunc")();
        context.get<std::string, detail::Closure<void(void)>>("increseValue")();
        std::cout << context.get<std::string, int>("value") << std::endl;

        auto increseValue = context.get<std::string, detail::Closure<void(void)>>("increseValue");
        auto bound = increseValue.get<std::string, detail::NativeClosure<decltype(increseValue)(SQObjectPtr)>>("bindenv")(anotherContext.holder->GetSQObjectPtr());
        bound();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception:: " << e.what() << '\n';
    }
    std::cout << "=========" << std::endl;
    try {
        auto rt = vm.getroottable();
        rt->get<std::string, detail::NativeClosure<void(std::string)>>("print")("test\n");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception:: " << e.what() << '\n';
    }
    std::cin.get();
}