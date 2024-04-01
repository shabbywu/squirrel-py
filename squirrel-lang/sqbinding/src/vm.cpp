#include "vm.h"
#include "compiler.h"
#include "types/container.h"
#include "types/object.h"
#include "types/sqiterator.h"


static const unsigned char LIB_IO   = 0x01;                                              ///< Input/Output library
static const unsigned char LIB_BLOB = 0x02;                                              ///< Blob library
static const unsigned char LIB_MATH = 0x04;                                              ///< Math library
static const unsigned char LIB_SYST = 0x08;                                              ///< System library
static const unsigned char LIB_STR  = 0x10;                                              ///< String library
static const unsigned char LIB_ALL  = LIB_IO | LIB_BLOB | LIB_MATH | LIB_SYST | LIB_STR; ///< All libraries

HSQUIRRELVM open_sqvm(int size, unsigned char libsToLoad) {
        HSQUIRRELVM vm = sq_open(size);
        sq_setprintfunc(vm, printStdout, printStdErr);
        sq_setcompilererrorhandler(vm, printCompileError);
        sq_pushroottable(vm);
        if (libsToLoad & LIB_IO)
            sqstd_register_iolib(vm);
        if (libsToLoad & LIB_BLOB)
            sqstd_register_bloblib(vm);
        if (libsToLoad & LIB_MATH)
            sqstd_register_mathlib(vm);
        if (libsToLoad & LIB_SYST)
            sqstd_register_systemlib(vm);
        if (libsToLoad & LIB_STR)
            sqstd_register_stringlib(vm);
        return vm;
}


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


SQInteger read_string(SQUserPointer p,SQUserPointer buffer,SQInteger size)
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


void StaticVM::DumpStack(SQInteger stackbase = -1, bool dumpall = false) {
    vm->dumpstack(stackbase, dumpall);
}

SQInteger StaticVM::gettop() {
    return sq_gettop(vm);
}

void StaticVM::settop(SQInteger top) {
    return sq_settop(vm, top);
}

std::shared_ptr<_SQTable_> StaticVM::getroottable() {
    if (roottable == NULL) {
        roottable = std::make_shared<_SQTable_>(_SQTable_(_table(vm->_roottable), vm));
    }
    return roottable;
}

void StaticVM::setroottable(std::shared_ptr<_SQTable_> roottable) {
    vm->_roottable = roottable.get();
    roottable = NULL;
}


PyValue StaticVM::ExecuteString(std::string sourcecode, PyValue env) {
    SQInteger oldtop = sq_gettop(vm);

    std::cout << "code: " << sourcecode << std::endl;
    if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
        std::cout << "invalid sourcecode, failed to compile" << std::endl;
        throw py::value_error("invalid sourcecode, failed to compile");
    }

    auto func = std::get<std::shared_ptr<_SQClosure_>>(sqobject_topython(vm->PopGet(), vm));
    if (!std::holds_alternative<py::none>(env)) {
        func->pthis = pyvalue_tosqobject(env, vm);
    }
    return func->__call__(py::list());
}

PyValue StaticVM::ExecuteBytecode(std::string bytecode, PyValue env) {
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

_SQObjectPtr_* StaticVM::StackTop() {
    return new _SQObjectPtr_(vm->Top(), this->vm, true);
};

void StaticVM::bindFunc(std::string funcname, py::function func) {
    _SQTable_(_table(vm->_roottable), vm).bindFunc(funcname, func);
}


class GenericVM: public StaticVM {
public:
    static const unsigned char LIB_IO   = 0x01;                                              ///< Input/Output library
    static const unsigned char LIB_BLOB = 0x02;                                              ///< Blob library
    static const unsigned char LIB_MATH = 0x04;                                              ///< Math library
    static const unsigned char LIB_SYST = 0x08;                                              ///< System library
    static const unsigned char LIB_STR  = 0x10;                                              ///< String library
    static const unsigned char LIB_ALL  = LIB_IO | LIB_BLOB | LIB_MATH | LIB_SYST | LIB_STR; ///< All libraries

    GenericVM(HSQUIRRELVM vm) { this->vm = vm; };
    GenericVM(): GenericVM(1024) {}
    GenericVM(int size): GenericVM(size, LIB_ALL) {}
    GenericVM(int size, unsigned char libsToLoad) {
        if (size <= 10) {
            throw py::value_error("stacksize can't less than 10");
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
        py::module::import("gc").attr("collect")();
        sq_close(vm);
    }
};


std::shared_ptr<StaticVM> static_vm;
void set_static_vm(HSQUIRRELVM vm) {
    if (static_vm == NULL) {
        static_vm = std::shared_ptr<StaticVM>(new StaticVM(vm), [](StaticVM* static_vm) {
            delete(static_vm);
        });
    }
}

std::shared_ptr<StaticVM> get_static_vm() {
    return static_vm;
}



void register_squirrel_vm(py::module_ &m) {
    py::class_<StaticVM, std::shared_ptr<StaticVM>>(m, "StaticVM")
        .def_readonly("vm", &StaticVM::vm)
        .def("dumpstack", &StaticVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def("execute", &StaticVM::ExecuteString, py::arg("sourcecode"), py::arg("env").none(true) = py::none())
        .def("execute_bytecode", &StaticVM::ExecuteBytecode, py::arg("bytecode"), py::arg("env").none(true) = py::none())
        .def("bindfunc", &StaticVM::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("stack_top", &StaticVM::StackTop, py::return_value_policy::reference_internal)
        // base api
        .def_property("top", &StaticVM::gettop, &StaticVM::settop, py::return_value_policy::reference_internal)
        .def("get_roottable", &StaticVM::getroottable, py::keep_alive<0, 1>())
        .def("set_roottable", &StaticVM::setroottable, py::arg("roottable"), py::keep_alive<1, 2>())
        .def("collectgarbage", [](StaticVM* vm) -> int {
            return sq_collectgarbage(vm->vm);
        })
        ;

    py::class_<GenericVM, std::shared_ptr<GenericVM>>(m, "SQVM")
        .def(py::init<int>(), py::arg("size") = 1024)
        .def_readonly("vm", &GenericVM::vm)
        .def("dumpstack", &GenericVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def("execute", &GenericVM::ExecuteString, py::arg("sourcecode"), py::arg("env").none(true) = py::none())
        .def("execute_bytecode", &GenericVM::ExecuteBytecode, py::arg("bytecode"), py::arg("env").none(true) = py::none())
        .def("bindfunc", &GenericVM::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("stack_top", &GenericVM::StackTop, py::return_value_policy::take_ownership)
        // base api
        .def_property("top", &GenericVM::gettop, &GenericVM::settop, py::return_value_policy::reference_internal)
        .def("get_roottable", &GenericVM::getroottable, py::keep_alive<0, 1>(), py::return_value_policy::move)
        .def("set_roottable", &GenericVM::setroottable, py::arg("roottable"), py::keep_alive<1, 2>())
        .def("collectgarbage", [](GenericVM* vm) -> int {
            return sq_collectgarbage(vm->vm);
        })
        ;

    m.def("compile", &compile, py::arg("sourcecode").none(false), py::arg("sourcename") = "__main__");
    m.def("get_static_vm", &get_static_vm);

    m.attr("SQUIRREL_VERSION") = SQUIRREL_VERSION;
}
