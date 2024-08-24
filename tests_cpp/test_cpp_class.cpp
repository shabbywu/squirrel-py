#include <iostream>
#include <string>
#include <type_traits>
#include <concepts>
#include <functional>
// #define TRACE_CONTAINER_GC
// #define TRACE_OBJECT_CAST
#include <sqbinding/detail/vm/vm.hpp>
#include <sqbinding/detail/types/sqclass.hpp>
#include <sqbinding/detail/common/cast_object.hpp>


class A{
    int i = 1;
    public:
        int field;

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
    auto vm = sqbinding::detail::GenericVM();
    std::cout << "=========" << std::endl;
    {
        sqbinding::detail::ClassDef<A> class_a(vm.GetVM(), "A");
        class_a.bindFunc("nonconst_method", &A::nonconst_method)->defProperty("field", &A::field);
        // class_a.set(std::string("attribute"), 2);
    }
    std::cout << "=========" << std::endl;
    try
    {
        auto rt = vm.getroottable();
        A a;
        a.field = 10086;
        rt->set(std::string("a"), &a);
        std::cout << "--------- call a.nonconst_method() start" << std::endl;
        vm.ExecuteString("print(typeof a); a.nonconst_method()");
        A* a_ptr = vm.getroottable()->get<std::string, A*>(std::string("a"));
        a_ptr->nonconst_method();
        std::cout << "========= call a.nonconst_method() end" << std::endl;

        std::cout << "--------- access a.field " << std::endl;
        vm.ExecuteString("print(a.field + \"\\n\")");
        vm.ExecuteString("a.field = 222");
        vm.ExecuteString("print(a.field + \"\\n\")");
        std::cout << a.field << std::endl;
        std::cout << "========= access a.field " << std::endl;

        // TODO: 支持设置实例属性 (get/set table)
        // std::cout << "--------- set a.attribute " << std::endl;
        // vm.ExecuteString("a.attribute = 3; print(a.attribute)");
        // std::cout << "========= set a.attribute " << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "=========" << std::endl;
    std::cin.get();
}
