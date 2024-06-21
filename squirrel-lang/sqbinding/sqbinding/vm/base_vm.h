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
#include "sqbinding/types/pybinding/definition.h"
#include "sqbinding/types/pybinding/container.h"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/common/cast.h"
#include "printer.h"

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
                struct Holder {
                    Holder(HSQUIRRELVM vm) : vm(vm) {
                        vmlock::register_vm_handle(vm);
                        roottable = std::make_shared<sqbinding::python::Table>(_table(vm->_roottable), vm);
                    }
                    ~Holder(){
                        #ifdef TRACE_CONTAINER_GC
                        std::cout << "GC::Release BaseVM: " << vm << std::endl;
                        #endif
                        vmlock::unregister_vm_handle(vm);
                    }
                    HSQUIRRELVM vm;
                    std::shared_ptr<sqbinding::python::Table> roottable;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                BaseVM() = default;
                BaseVM(HSQUIRRELVM vm) : holder(std::make_shared<Holder>(vm)) {}
            public:
                std::shared_ptr<sqbinding::python::Table> getroottable() {
                    return holder->roottable;
                }
                std::shared_ptr<sqbinding::python::ObjectPtr> StackTop() {
                    HSQUIRRELVM& vm = holder->vm;
                    return std::make_shared<sqbinding::python::ObjectPtr>(vm->Top(), vm);
                }
                HSQUIRRELVM& GetVM() {return holder->vm;}

                inline std::string GetLastError() {
                    HSQUIRRELVM& vm = holder->vm;
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
                    holder->vm->dumpstack(stackbase, dumpall);
                }
                SQInteger GetTop() {
                    return sq_gettop(holder->vm);
                }
                void SetTop(SQInteger top) {
                    return sq_settop(holder->vm, top);
                }
            public:
            template <class Return, class Env>
            std::remove_reference_t<Return> ExecuteString(std::string sourcecode, Env& env) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = holder->vm;
                detail::stack_guard stack_guard(vm);

                if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
                    throw sqbinding::value_error("invalid sourcecode, failed to compile");
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), vm};
                SQObjectPtr pthis = generic_cast<std::remove_reference_t<Env>, SQObjectPtr>(vm, env);
                closure.bindThis(pthis);
                return closure();
            }

            template <class Return>
            std::remove_reference_t<Return> ExecuteString(std::string sourcecode) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = holder->vm;
                detail::stack_guard stack_guard(vm);

                if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
                    throw sqbinding::value_error("invalid sourcecode, failed to compile");
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), vm};
                return closure();
            }
            public:
            template <class Return, class Env>
            std::remove_reference_t<Return> ExecuteBytecode(std::string bytecode, Env& env) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = holder->vm;
                detail::stack_guard stack_guard(vm);

                auto reader = StringReaderCtx(bytecode);
                if (!SQ_SUCCEEDED(sq_readclosure(vm, StringReaderCtx::read_string, &reader))) {
                    throw std::runtime_error(GetLastError());
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), vm};
                SQObjectPtr pthis = generic_cast<std::remove_reference_t<Env>, SQObjectPtr>(vm, env);
                closure.bindThis(pthis);
                return closure();
            }

            template <class Return>
            std::remove_reference_t<Return> ExecuteBytecode(std::string bytecode) {
                typedef Closure<std::remove_reference_t<Return> ()> ClosureType;
                HSQUIRRELVM& vm = holder->vm;
                detail::stack_guard stack_guard(vm);

                auto reader = StringReaderCtx(bytecode);
                if (!SQ_SUCCEEDED(sq_readclosure(vm, StringReaderCtx::read_string, &reader))) {
                    throw std::runtime_error(GetLastError());
                }

                ClosureType closure = ClosureType{_closure(vm->Top()), vm};
                return closure();
            }
        };
    }

    namespace python {
        class BaseVM: public detail::BaseVM {
        public:
            BaseVM(): detail::BaseVM() {}
            BaseVM(HSQUIRRELVM vm): detail::BaseVM(vm) {}

            void bindFunc(std::string funcname, py::function func) {
                HSQUIRRELVM& vm = holder->vm;
                holder->roottable->bindFunc(funcname, func);
            }
        };
    }
}
