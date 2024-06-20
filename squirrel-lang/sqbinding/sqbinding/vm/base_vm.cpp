#include "base_vm.h"
#include "compiler.h"
#include "sqbinding/types/container.h"
#include "sqbinding/types/object.h"
#include "sqbinding/types/sqiterator.h"

namespace py = pybind11;

static class StringReaderCtx {
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


void BaseVM::DumpStack(SQInteger stackbase = -1, bool dumpall = false) {
    vm->dumpstack(stackbase, dumpall);
}

SQInteger BaseVM::gettop() {
    return sq_gettop(vm);
}

void BaseVM::settop(SQInteger top) {
    return sq_settop(vm, top);
}

std::shared_ptr<_SQTable_> BaseVM::getroottable() {
    if (roottable == NULL) {
        roottable = std::make_shared<_SQTable_>(_SQTable_(_table(vm->_roottable), vm));
    }
    return roottable;
}

void BaseVM::setroottable(std::shared_ptr<_SQTable_> roottable) {
    vm->_roottable = roottable.get();
    roottable = NULL;
}


PyValue BaseVM::ExecuteString(std::string sourcecode, PyValue env) {
    SQInteger oldtop = sq_gettop(vm);

    if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
        throw py::value_error("invalid sourcecode, failed to compile");
    }

    auto func = std::get<std::shared_ptr<_SQClosure_>>(sqobject_topython(vm->PopGet(), vm));
    if (!std::holds_alternative<py::none>(env)) {
        func->pthis = pyvalue_tosqobject(env, vm);
    }
    return func->__call__(py::list());
}

PyValue BaseVM::ExecuteBytecode(std::string bytecode, PyValue env) {
    auto reader = StringReaderCtx(bytecode);
    SQInteger oldtop = sq_gettop(vm);
    if (!SQ_SUCCEEDED(sq_readclosure(vm, read_string, &reader))) {
        throw std::runtime_error(GetLastError());
    }
    auto func = std::get<std::shared_ptr<_SQClosure_>>(sqobject_topython(vm->PopGet(), vm));
    if (!std::holds_alternative<py::none>(env)) {
        func->pthis = pyvalue_tosqobject(env, vm);
    }
    return func->__call__(py::list());
}

_SQObjectPtr_* BaseVM::StackTop() {
    return new _SQObjectPtr_(vm->Top(), this->vm, true);
};

void BaseVM::bindFunc(std::string funcname, py::function func) {
    _SQTable_(_table(vm->_roottable), vm).bindFunc(funcname, func);
}
