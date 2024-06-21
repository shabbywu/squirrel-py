#include "base_vm.h"
#include "compiler.h"
#include "sqbinding/types/container.h"

namespace py = pybind11;

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
};


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


void sqbinding::detail::BaseVM::DumpStack(SQInteger stackbase = -1, bool dumpall = false) {
    holder->vm->dumpstack(stackbase, dumpall);
}

SQInteger sqbinding::detail::BaseVM::gettop() {
    return sq_gettop(holder->vm);
}

void sqbinding::detail::BaseVM::settop(SQInteger top) {
    return sq_settop(holder->vm, top);
}

std::shared_ptr<sqbinding::python::Table> sqbinding::detail::BaseVM::getroottable() {
    return holder->roottable;
}

std::shared_ptr<sqbinding::python::ObjectPtr> sqbinding::detail::BaseVM::StackTop() {
    HSQUIRRELVM& vm = holder->vm;
    return std::make_shared<sqbinding::python::ObjectPtr>(vm->Top(), vm);
};

PyValue sqbinding::python::BaseVM::ExecuteString(std::string sourcecode, PyValue env) {
    HSQUIRRELVM& vm = holder->vm;
    detail::stack_guard stack_guard(vm);

    if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
        throw py::value_error("invalid sourcecode, failed to compile");
    }
    auto func = std::get<std::shared_ptr<sqbinding::python::Closure>>(sqobject_topython(vm->Top(), vm));
    if (!std::holds_alternative<py::none>(env)) {
        func->pthis = pyvalue_tosqobject(env, vm);
    }
    return func->__call__(py::list());
}

PyValue sqbinding::python::BaseVM::ExecuteBytecode(std::string bytecode, PyValue env) {
    HSQUIRRELVM& vm = holder->vm;
    detail::stack_guard stack_guard(vm);
    auto reader = StringReaderCtx(bytecode);
    if (!SQ_SUCCEEDED(sq_readclosure(vm, read_string, &reader))) {
        throw std::runtime_error(GetLastError());
    }
    auto func = std::get<std::shared_ptr<sqbinding::python::Closure>>(sqobject_topython(vm->Top(), vm));
    if (!std::holds_alternative<py::none>(env)) {
        func->pthis = pyvalue_tosqobject(env, vm);
    }
    return func->__call__(py::list());
}

void sqbinding::python::BaseVM::bindFunc(std::string funcname, py::function func) {
    HSQUIRRELVM& vm = holder->vm;
    holder->roottable->bindFunc(funcname, func);
}
