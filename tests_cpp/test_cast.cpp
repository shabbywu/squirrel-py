#include <concepts>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/common/cast_object.hpp>
#include <sqbinding/detail/types/sqfunction.hpp>
#include <sqbinding/detail/vm/vm.hpp>

using namespace sqbinding;

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
    }
    std::cout << "=========" << std::endl;
    std::cin.get();
}
