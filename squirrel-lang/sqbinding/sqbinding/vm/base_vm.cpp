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


void sqbinding::python::BaseVM::DumpStack(SQInteger stackbase = -1, bool dumpall = false) {
    vm->dumpstack(stackbase, dumpall);
}

SQInteger sqbinding::python::BaseVM::gettop() {
    return sq_gettop(vm);
}

void sqbinding::python::BaseVM::settop(SQInteger top) {
    return sq_settop(vm, top);
}

std::shared_ptr<sqbinding::python::Table> sqbinding::python::BaseVM::getroottable() {
    if (roottable == NULL) {
        roottable = std::make_shared<sqbinding::python::Table>(sqbinding::python::Table(_table(vm->_roottable), vm));
    }
    return roottable;
}

void sqbinding::python::BaseVM::setroottable(std::shared_ptr<sqbinding::python::Table> roottable) {
    vm->_roottable = roottable.get();
    roottable = NULL;
}


PyValue sqbinding::python::BaseVM::ExecuteString(std::string sourcecode, PyValue env) {
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

std::shared_ptr<sqbinding::python::ObjectPtr> sqbinding::python::BaseVM::StackTop() {
    return std::make_shared<sqbinding::python::ObjectPtr>(vm->Top(), vm);
};

void sqbinding::python::BaseVM::bindFunc(std::string funcname, py::function func) {
    sqbinding::python::Table(_table(vm->_roottable), vm).bindFunc(funcname, func);
}
