#include <iostream>
#include <sqbinding/detail/sqdifinition.hpp>
#include <sqbinding/detail/vm/vm.hpp>


void main(){
    auto vm = sqbinding::detail::GenericVM();
    vm.ExecuteString<void>(R"(
        print("HelloWorld\n")
    )");
    std::cin.get();
}
