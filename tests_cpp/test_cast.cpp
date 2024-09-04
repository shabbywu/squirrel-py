#include <concepts>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>

// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/cast.hpp>
#include <sqbinding/detail/types/holder.hpp>
#include <sqbinding/detail/types/sqfunction.hpp>
#include <sqbinding/detail/vm/vm.hpp>

using namespace sqbinding;

class A {};

void main() {
    {
        auto vm = detail::GenericVM();
        vm.ExecuteString(R"(
        local rt = getroottable();
        rt.voidFunc <- function () {
            print("call void(void) func\n");
        };
    )");
        auto rt = *vm.getroottable();
        std::cout << "=========" << std::endl;
        try {
            rt.get<std::string, detail::Closure<void(void)>>(std::string("voidFunc"))();
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
        SQObjectPtr a;
        std::shared_ptr<detail::SQObjectPtrHolder<::SQObjectPtr>> p =
            std::make_shared<detail::SQObjectPtrHolder<::SQObjectPtr>>(a, vm.GetVM());
    }

    std::cout << "=========" << std::endl;
    std::cin.get();
}
