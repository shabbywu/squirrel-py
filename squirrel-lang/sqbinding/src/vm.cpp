#include "vm.h"
#include "compiler.h"
#include "types/sqcontainer.h"
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
    return std::make_shared<_SQTable_>(_SQTable_(_table(vm->_roottable), vm));
}

void StaticVM::setroottable(std::shared_ptr<_SQTable_> roottable) {
    vm->_roottable = roottable.get();
}


void StaticVM::ExecuteString(std::string sourcecode) {
    SQInteger oldtop = sq_gettop(vm);

    if (!SQ_SUCCEEDED(sq_compilebuffer(vm, sourcecode.c_str(), sourcecode.length(), "__main__", false))) {
        throw py::value_error("invalid sourcecode, failed to compile");
    }
    sq_pushroottable(vm);
    SQRESULT result = sq_call(vm, 1, false, true);
    sq_settop(vm, oldtop);
    if (SQ_FAILED(result)) {
        throw std::runtime_error(GetLastError());
    }
}

void StaticVM::ExecuteBytecode(std::string bytecode) {
    auto reader = StringReaderCtx(bytecode);
    SQInteger oldtop = sq_gettop(vm);
    if (!SQ_SUCCEEDED(sq_readclosure(vm, read_string, &reader))) {
        throw std::runtime_error(GetLastError());
    }
    sq_pushroottable(vm);
    SQRESULT result = sq_call(vm, 1, false, true);
    sq_settop(vm, oldtop);
    if (SQ_FAILED(result)) {
        throw std::runtime_error(GetLastError());
    }
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
    }
    ~GenericVM() {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release GenericVM" << std::endl;
        #endif
        sq_collectgarbage(vm);
        sq_settop(vm, 0);
        py::module::import("gc").attr("collect")();
        sq_close(vm);
    }
};


void register_squirrel_vm(py::module_ &m) {
    py::class_<StaticVM, std::shared_ptr<StaticVM>>(m, "StaticVM")
        .def_readonly("vm", &StaticVM::vm)
        .def("dumpstack", &StaticVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def("execute", &StaticVM::ExecuteString, py::arg("sourcecode"))
        .def("execute_bytecode", &StaticVM::ExecuteBytecode, py::arg("bytecode"))
        .def("bindfunc", &StaticVM::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("stack_top", &StaticVM::StackTop, py::return_value_policy::reference_internal)
        // base api
        .def_property("top", &StaticVM::gettop, &StaticVM::settop, py::return_value_policy::reference_internal)
        .def("get_roottable", &StaticVM::getroottable, py::keep_alive<0, 1>())
        .def("set_roottable", &StaticVM::setroottable, py::arg("roottable"), py::keep_alive<1, 2>())
        ;

    py::class_<GenericVM, std::shared_ptr<GenericVM>>(m, "SQVM")
        .def(py::init<int>(), py::arg("size") = 1024)
        .def_readonly("vm", &GenericVM::vm)
        .def("dumpstack", &GenericVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def("execute", &GenericVM::ExecuteString, py::arg("sourcecode"))
        .def("execute_bytecode", &GenericVM::ExecuteBytecode, py::arg("bytecode"))
        .def("bindfunc", &GenericVM::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("stack_top", &GenericVM::StackTop, py::return_value_policy::take_ownership)
        // base api
        .def_property("top", &GenericVM::gettop, &GenericVM::settop, py::return_value_policy::reference_internal)
        .def("get_roottable", &GenericVM::getroottable, py::keep_alive<0, 1>(), py::return_value_policy::move)
        .def("set_roottable", &GenericVM::setroottable, py::arg("roottable"), py::keep_alive<1, 2>())
        ;

    m.def("compile", &compile, py::arg("sourcecode").none(false), py::arg("sourcename") = "__main__");
}
