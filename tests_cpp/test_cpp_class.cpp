#include <concepts>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/common/cast_object.hpp>
#include <sqbinding/detail/types/sqclass.hpp>
#include <sqbinding/detail/vm/vm.hpp>

using namespace sqbinding;

class A {
    int i = 1;

  public:
    int field;

  public:
    static void static_method() {
        std::cout << "Hello static_method" << std::endl;
    }

    void nonconst_method() {
        i++;
        std::cout << "Hello nonconst class method: " << i << std::endl;
    }

    void const_method() const {
        std::cout << "Hello const class method: " << i << std::endl;
    }
};

void test() {
    auto vm = detail::GenericVM();
    std::cout << "[*] step: binding cpp method" << std::endl;
    {
        detail::ClassDef<A> class_a(vm.GetVM(), "A");
        class_a.bindFunc("static_method", &A::static_method)
            .bindFunc("nonconst_method", &A::nonconst_method)
            .bindFunc("const_method", &A::const_method)
            .defProperty("field", &A::field);
        // class_a.set(std::string("attribute"), 2);
    }

    std::cout << "=========" << std::endl;
    // try {
    //     auto rt = vm.getroottable();
    //     A a1;
    //     A a2;
    //     a1.field = 10086;
    //     a2.field = 999;
    //     rt->set(std::string("a"), &a1);
    //     vm.ExecuteString("print(\"typeof a = \" + typeof a + \"\\n\"); ");
    //     std::cout << "[*] calling A::static_method" << std::endl;
    //     vm.ExecuteString("a.static_method()");

    //     std::cout << "[*] calling A::nonconst_method" << std::endl;
    //     vm.ExecuteString("a.nonconst_method()");

    //     std::cout << "[*] calling A::const_method" << std::endl;
    //     vm.ExecuteString("a.const_method()");

    //     std::cout << "[*] calling A::nonconst_method via pointer from vm" << std::endl;
    //     A *a_ptr = vm.getroottable()->get<std::string, A *>(std::string("a"));
    //     a_ptr->nonconst_method();

    //     std::cout << "[*] accessing a.field " << std::endl;
    //     vm.ExecuteString("print(\"a.field=\" + a.field + \"\\n\")");
    //     vm.ExecuteString("a.field = 222");
    //     vm.ExecuteString("print(\"a.field=\" + a.field + \"\\n\")");
    //     std::cout << a1.field << std::endl;

    //     std::cout << "[*] newslot a.attribute " << std::endl;
    //     vm.ExecuteString("a.attribute = 3;");
    //     vm.ExecuteString("print(\"a.attribute=\" + a.attribute + \"\\n\")");

    //     rt->set(std::string("a2"), &a2);
    //     std::cout << "[*] accessing a2.field " << std::endl;
    //     vm.ExecuteString("print(\"a2.field=\" + a2.field + \"\\n\")");

    //     std::cout << "[*] accessing a2.attribute " << std::endl;
    //     vm.ExecuteString("print(\"a2.attribute=\" + a2.attribute + \"\\n\")");
    // }
    // catch (const std::exception &e) {
    //     std::cerr << e.what() << '\n';
    // }
    detail::ClassRegistry::instances.clear();
    _sleep(1000);
    std::cout << "=========" << std::endl;
}

void main() {
    test();
    std::cin.get();
}
