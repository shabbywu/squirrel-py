#include "base_vm.h"
#include "sqbinding/common/errors.h"

enum class SquirrelLibs {
    LIB_IO   = 0b1,
    LIB_BLOB = 0b10,
    LIB_MATH = 0b100,
    LIB_SYST = 0b1000,
    LIB_STR = 0b10000,
    LIB_ALL  = LIB_IO | LIB_BLOB | LIB_MATH | LIB_SYST | LIB_STR,
};


HSQUIRRELVM open_sqvm(int size, unsigned int libsToLoad);


class GenericVM: public BaseVM {
public:
    static const unsigned char LIB_IO   = 0x01;                                              ///< Input/Output library
    static const unsigned char LIB_BLOB = 0x02;                                              ///< Blob library
    static const unsigned char LIB_MATH = 0x04;                                              ///< Math library
    static const unsigned char LIB_SYST = 0x08;                                              ///< System library
    static const unsigned char LIB_STR  = 0x10;                                              ///< String library
    static const unsigned char LIB_ALL  = LIB_IO | LIB_BLOB | LIB_MATH | LIB_SYST | LIB_STR; ///< All libraries

    GenericVM(HSQUIRRELVM vm): BaseVM(vm) {};
    GenericVM(): GenericVM(1024) {}
    GenericVM(int size): GenericVM(size, LIB_ALL) {}
    GenericVM(int size, unsigned char libsToLoad) {
        if (size <= 10) {
            throw sqbinding::value_error("stacksize can't less than 10");
        }
        vm = open_sqvm(size, libsToLoad);
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
