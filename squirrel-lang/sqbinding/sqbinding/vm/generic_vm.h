#include "base_vm.h"
#include "sqbinding/common/errors.h"
#include "sqbinding/common/cast.h"






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
    }

    namespace python {
        class GenericVM: public BaseVM {
        public:
            GenericVM(HSQUIRRELVM vm): BaseVM(vm) {};
            GenericVM(): GenericVM(1024) {}
            GenericVM(int size): GenericVM(size, (unsigned int)detail::SquirrelLibs::LIB_ALL) {}
            GenericVM(int size, unsigned int libsToLoad) {
                if (size <= 10) {
                    throw sqbinding::value_error("stacksize can't less than 10");
                }
                vm = detail::open_sqvm(size, libsToLoad);
                vmlock::register_vm_handle(vm);
            }

            virtual void release() {
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release GenericVM step1" << std::endl;
                #endif
                roottable = NULL;
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release GenericVM step2" << std::endl;
                #endif
                vmlock::unregister_vm_handle(vm);
                sq_collectgarbage(vm);
                sq_settop(vm, 0);
                sq_close(vm);
            }
        };
    }
}
