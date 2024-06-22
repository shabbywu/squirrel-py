#pragma once
#include <iostream>
#include <cstring>
#include <memory.h>

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sqbinding/detail/types/sqtable.hpp"
#include "sqbinding/detail/types/sqobject.hpp"
#include "sqbinding/detail/types/sqvm.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/cast.hpp"
#include "sqbinding/detail/vm/printer.hpp"

namespace py = pybind11;
namespace sqbinding {
    namespace detail {
        class StringReaderCtx {
        public:
            const char* c_str;
            int readed;
            int length;

            StringReaderCtx(std::string &string) {
                this->c_str = string.c_str();
                this->readed = 0;
                this->length = string.length();
            }

            static SQInteger read_string(SQUserPointer p,SQUserPointer buffer,SQInteger size)
            {
                auto ctx = (StringReaderCtx*) p;
                if (ctx->readed + size <= ctx->length) {
                    for (int i = 0; i < size; i++) {
                        *((char*)(buffer) + i) = *(ctx->c_str + ctx->readed + i);
                    }
                    ctx->readed += size;
                    return size;
                }
                return -1;
            }
        };

        class BaseVM {
            public:
                std::shared_ptr<detail::VM> holder;
                std::shared_ptr<detail::Table> roottable;
            public:
                BaseVM() = default;
                BaseVM(HSQUIRRELVM vm) : BaseVM(vm, false) {}
                BaseVM(HSQUIRRELVM vm, bool should_close) : holder(std::make_shared<detail::VM>(vm, should_close)) {}
            public:
                HSQUIRRELVM& GetSQVM() {return **holder;}
                VM& GetVM() {return *holder;}
                std::shared_ptr<detail::Table>& getroottable() {
                    if (roottable == nullptr) {
                        roottable = std::make_shared<detail::Table>(_table(GetSQVM()->_roottable), GetVM());
                    }
                    return roottable;
                }
                std::shared_ptr<detail::ObjectPtr> StackTop() {
                    return std::make_shared<detail::ObjectPtr>(GetSQVM()->Top(), GetVM());
                }

                inline std::string GetLastError() {
                    HSQUIRRELVM& vm = GetSQVM();
                    const SQChar* sqErr;
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
            public:
            template <class Return, class Env>
            std::remove_reference_t<Return> ExecuteString(std::string sourcecode, Env& env) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = GetSQVM();
                detail::stack_guard stack_guard(vm);

                if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
                    throw sqbinding::value_error("invalid sourcecode, failed to compile");
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), GetVM()};
                SQObjectPtr pthis = detail::generic_cast<std::remove_reference_t<Env>, SQObjectPtr>(GetVM(), env);
                closure.bindThis(pthis);
                return closure();
            }

            template <class Return>
            std::remove_reference_t<Return> ExecuteString(std::string sourcecode) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = GetSQVM();
                detail::stack_guard stack_guard(vm);

                if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
                    throw sqbinding::value_error("invalid sourcecode, failed to compile");
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), GetVM()};
                return closure();
            }
            public:
            template <class Return, class Env>
            std::remove_reference_t<Return> ExecuteBytecode(std::string bytecode, Env& env) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = GetSQVM();
                detail::stack_guard stack_guard(vm);

                auto reader = StringReaderCtx(bytecode);
                if (!SQ_SUCCEEDED(sq_readclosure(vm, StringReaderCtx::read_string, &reader))) {
                    throw std::runtime_error(GetLastError());
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), GetVM()};
                SQObjectPtr pthis = detail::generic_cast<std::remove_reference_t<Env>, SQObjectPtr>(GetVM(), env);
                closure.bindThis(pthis);
                return closure();
            }

            template <class Return>
            std::remove_reference_t<Return> ExecuteBytecode(std::string bytecode) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = GetSQVM();
                detail::stack_guard stack_guard(vm);

                auto reader = StringReaderCtx(bytecode);
                if (!SQ_SUCCEEDED(sq_readclosure(vm, StringReaderCtx::read_string, &reader))) {
                    throw std::runtime_error(GetLastError());
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), GetVM()};
                return closure();
            }
        };
    }
}
