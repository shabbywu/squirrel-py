#include "base_vm.h"
#include "sqbinding/common/errors.h"
#include "sqbinding/common/cast.h"
#include "printer.h"


namespace sqbinding {
    namespace detail {
        enum class SquirrelLibs {
            LIB_IO   = 0b1,                                                 ///< Input/Output library
            LIB_BLOB = 0b10,                                                ///< Blob library
            LIB_MATH = 0b100,                                               ///< Math library
            LIB_SYST = 0b1000,                                              ///< System library
            LIB_STR = 0b10000,                                              ///< String library
            LIB_ALL  = LIB_IO | LIB_BLOB | LIB_MATH | LIB_SYST | LIB_STR,   ///< All libraries
        };

        HSQUIRRELVM open_sqvm(int size, unsigned int libsToLoad);

        class GenericVM: public detail::BaseVM {
            public:
                struct Holder: BaseVM::Holder {
                    Holder(HSQUIRRELVM vm) : BaseVM::Holder(vm) {
                    };
                    ~Holder(){
                        #ifdef TRACE_CONTAINER_GC
                        std::cout << "GC::Release GenericVM: " << vm << std::endl;
                        #endif
                        vmlock::unregister_vm_handle(vm);
                        sq_collectgarbage(vm);
                        sq_settop(vm, 0);
                        sq_close(vm);
                    }
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                GenericVM(HSQUIRRELVM vm): BaseVM(vm) {};
                GenericVM(): GenericVM(1024) {}
                GenericVM(int size): GenericVM(size, (unsigned int)detail::SquirrelLibs::LIB_ALL) {}
                GenericVM(int size, unsigned int libsToLoad): BaseVM(detail::open_sqvm(size, libsToLoad)) {}
        };
    }

    namespace python {
        class GenericVM: public detail::GenericVM {
        public:
            GenericVM(HSQUIRRELVM vm): detail::GenericVM(vm) {};
            GenericVM(): detail::GenericVM(1024) {}
            GenericVM(int size): detail::GenericVM(size, (unsigned int)detail::SquirrelLibs::LIB_ALL) {}
            GenericVM(int size, unsigned int libsToLoad): detail::GenericVM(size, libsToLoad) {}

            void bindFunc(std::string funcname, py::function func) {
                HSQUIRRELVM& vm = holder->vm;
                holder->roottable->bindFunc(funcname, func);
            }
        };
    }
}
