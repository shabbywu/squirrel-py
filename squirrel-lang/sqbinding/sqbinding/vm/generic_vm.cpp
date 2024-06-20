#include "base_vm.h"
#include "generic_vm.h"

HSQUIRRELVM open_sqvm(int size, unsigned int libsToLoad) {
        HSQUIRRELVM vm = sq_open(size);
        sq_setprintfunc(vm, printStdout, printStdErr);
        sq_setcompilererrorhandler(vm, printCompileError);
        sq_pushroottable(vm);
        if (libsToLoad & (unsigned int)SquirrelLibs::LIB_IO)
            sqstd_register_iolib(vm);
        if (libsToLoad & (unsigned int)SquirrelLibs::LIB_BLOB)
            sqstd_register_bloblib(vm);
        if (libsToLoad & (unsigned int)SquirrelLibs::LIB_MATH)
            sqstd_register_mathlib(vm);
        if (libsToLoad & (unsigned int)SquirrelLibs::LIB_SYST)
            sqstd_register_systemlib(vm);
        if (libsToLoad & (unsigned int)SquirrelLibs::LIB_STR)
            sqstd_register_stringlib(vm);
        sq_poptop(vm);
        return vm;
}
