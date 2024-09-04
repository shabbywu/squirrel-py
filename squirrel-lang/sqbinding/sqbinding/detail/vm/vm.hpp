#pragma once
#include <cstring>
#include <iostream>
#include <memory.h>

#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/types/sqfunction.hpp"
#include "sqbinding/detail/types/sqobject.hpp"
#include "sqbinding/detail/types/sqtable.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/detail/vm/printer.hpp"
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include <squirrel.h>

namespace sqbinding {
namespace detail {
class StringReaderCtx {
  public:
    const char *c_str;
    int readed;
    int length;

    StringReaderCtx(std::string &string) {
        this->c_str = string.c_str();
        this->readed = 0;
        this->length = string.length();
    }

    static SQInteger read_string(SQUserPointer p, SQUserPointer buffer, SQInteger size) {
        auto ctx = (StringReaderCtx *)p;
        if (ctx->readed + size <= ctx->length) {
            for (int i = 0; i < size; i++) {
                *((char *)(buffer) + i) = *(ctx->c_str + ctx->readed + i);
            }
            ctx->readed += size;
            return size;
        }
        return -1;
    }
};
} // namespace detail

namespace detail {
class VMProxy {
  public:
    detail::VM vm;
    std::shared_ptr<detail::Table> roottable;

  public:
    VMProxy() = delete;
    VMProxy(HSQUIRRELVM vm) : VMProxy(vm, false) {
    }
    VMProxy(HSQUIRRELVM vm, bool should_close) : vm(vm, should_close) {
    }
    ~VMProxy() {
#ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release detail::VMProxy: " << GetSQVM() << std::endl;
#endif
        roottable = nullptr;
    }

  public:
    HSQUIRRELVM &GetSQVM() {
        return *vm;
    }
    detail::VM &GetVM() {
        return vm;
    }
    std::shared_ptr<detail::Table> &getroottable() {
        if (roottable == nullptr) {
            roottable = std::make_shared<detail::Table>(_table(GetSQVM()->_roottable), GetVM());
        }
        return roottable;
    }
    std::shared_ptr<detail::ObjectPtr> StackTop() {
        return std::make_shared<detail::ObjectPtr>(GetSQVM()->Top(), GetVM());
    }

    inline std::string GetLastError() {
        HSQUIRRELVM &vm = GetSQVM();
        const SQChar *sqErr;
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL) {
            sq_pop(vm, 1);
            return std::string();
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 2);
        return std::string(sqErr);
    };

  public:
    void DumpStack(SQInteger stackbase, bool dumpall) {
        GetSQVM()->dumpstack(stackbase, dumpall);
    }
    SQInteger GetTop() {
        return sq_gettop(GetSQVM());
    }
    void SetTop(SQInteger top) {
        return sq_settop(GetSQVM(), top);
    }

    SQInteger CollectGarbage() {
        return sq_collectgarbage(GetSQVM());
    }

  public:
    template <class Return, class Env> std::remove_reference_t<Return> ExecuteString(std::string sourcecode, Env &env) {
        typedef detail::Closure<std::remove_reference_t<Return>()> ClosureType;
        HSQUIRRELVM &vm = GetSQVM();
        detail::stack_guard stack_guard(vm);

        if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
            throw sqbinding::value_error("invalid sourcecode, failed to compile");
        }

        ClosureType closure{_closure(vm->Top()), GetVM()};
        SQObjectPtr pthis = detail::generic_cast<Env, SQObjectPtr>(GetVM(), std::forward<Env>(env));
        closure.bindThis(pthis);
        return closure();
    }

    template <class Return = void> std::remove_reference_t<Return> ExecuteString(std::string sourcecode) {
        typedef detail::Closure<std::remove_reference_t<Return>()> ClosureType;
        HSQUIRRELVM &vm = GetSQVM();
        detail::stack_guard stack_guard(vm);

        if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
            throw sqbinding::value_error("invalid sourcecode, failed to compile");
        }

        ClosureType closure{_closure(vm->Top()), GetVM()};
        return closure();
    }

  public:
    template <class Return, class Env> std::remove_reference_t<Return> ExecuteBytecode(std::string bytecode, Env &env) {
        typedef detail::Closure<std::remove_reference_t<Return>()> ClosureType;
        HSQUIRRELVM &vm = GetSQVM();
        detail::stack_guard stack_guard(vm);

        auto reader = detail::StringReaderCtx(bytecode);
        if (!SQ_SUCCEEDED(sq_readclosure(vm, detail::StringReaderCtx::read_string, &reader))) {
            throw std::runtime_error(GetLastError());
        }

        ClosureType closure{_closure(vm->Top()), GetVM()};
        SQObjectPtr pthis = detail::generic_cast<Env, SQObjectPtr>(GetVM(), std::forward<Env>(env));
        closure.bindThis(pthis);
        return closure();
    }

    template <class Return> std::remove_reference_t<Return> ExecuteBytecode(std::string bytecode) {
        typedef detail::Closure<std::remove_reference_t<Return>()> ClosureType;
        HSQUIRRELVM &vm = GetSQVM();
        detail::stack_guard stack_guard(vm);

        auto reader = detail::StringReaderCtx(bytecode);
        if (!SQ_SUCCEEDED(sq_readclosure(vm, detail::StringReaderCtx::read_string, &reader))) {
            throw std::runtime_error(GetLastError());
        }

        ClosureType closure{_closure(vm->Top()), GetVM()};
        return closure();
    }

    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        getroottable()->bindFunc(funcname, std::forward<Func>(func), withenv);
    }
};

enum class SquirrelLibs {
    LIB_IO = 0b1,                                                ///< Input/Output library
    LIB_BLOB = 0b10,                                             ///< Blob library
    LIB_MATH = 0b100,                                            ///< Math library
    LIB_SYST = 0b1000,                                           ///< System library
    LIB_STR = 0b10000,                                           ///< String library
    LIB_ALL = LIB_IO | LIB_BLOB | LIB_MATH | LIB_SYST | LIB_STR, ///< All libraries
};

static HSQUIRRELVM open_sqvm(int size, unsigned int libsToLoad) {
    if (size <= 10) {
        throw sqbinding::value_error("stacksize can't less than 10");
    }
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

class GenericVM : public VMProxy {
  public:
    GenericVM(HSQUIRRELVM vm) : VMProxy(vm) {};
    GenericVM() : GenericVM(1024) {
    }
    GenericVM(int size) : GenericVM(size, (unsigned int)detail::SquirrelLibs::LIB_ALL) {
    }
    GenericVM(int size, unsigned int libsToLoad) : detail::VMProxy(detail::open_sqvm(size, libsToLoad), true) {
    }
    ~GenericVM() {
#ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release detail::GenericVM: " << GetSQVM() << std::endl;
#endif
    }
};
} // namespace detail
} // namespace sqbinding
