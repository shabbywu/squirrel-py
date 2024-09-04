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

void test_getter(detail::GenericVM &vm) {
    detail::Array array(vm.GetVM());
    array.append(std::string("1"));
    array.append(1);
    array.set(0, std::string("-1"));
    auto v = array.get<int, std::string>(0);
    std::cout << "index[0]: " << v << std::endl;
}

void test_setter(detail::GenericVM &vm) {
    detail::Array array(vm.GetVM());
    array.append(std::string("1"));
    array.append(1);
    array.set(0, std::string("-1"));
    for (auto v : array.iterator()) {
        std::cout << "item: " << detail::sqobject_to_string(v) << std::endl;
    }
}

void main() {
    {
        auto vm = detail::GenericVM();
        test_getter(vm);
        std::cout << "==========" << std::endl;
        test_setter(vm);
    }
    std::cin.get();
}
