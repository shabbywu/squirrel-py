#include <iostream>
#include <string>
#include <sqbinding/detail/vm/vm.hpp>


void main(){
    auto vm = sqbinding::detail::GenericVM();
    // vm.bindFunc("words", [=]() -> std::string {
    //     return std::string("Hello World\n");
    // });
    vm.ExecuteString<void>(R"(
        print(words())
    )");
    std::cin.get();
}
