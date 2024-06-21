#include "sqbinding/vm/base_vm.h"
#include "sqbinding/vm/generic_vm.h"
#include "sqbinding/vm/compiler.h"
#include "sqbinding/types/container.h"

typedef sqbinding::python::BaseVM StaticVM;
typedef sqbinding::python::GenericVM GenericVM;


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
        .def_property_readonly("vm", &StaticVM::GetVM)
        .def("dumpstack", &StaticVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def("execute", [](StaticVM* vm, std::string sourcecode, PyValue env) -> PyValue {
            if (std::holds_alternative<py::none>(env)) {
                return vm->ExecuteString<PyValue>(sourcecode);
            }
            return vm->ExecuteString<PyValue, PyValue>(sourcecode, env);
        }, py::arg("sourcecode"), py::arg("env").none(true) = py::none())
        .def("execute_bytecode", [](StaticVM* vm, std::string bytecode, PyValue env) -> PyValue {
            if (std::holds_alternative<py::none>(env)) {
                return vm->ExecuteBytecode<PyValue>(bytecode);
            }
            return vm->ExecuteBytecode<PyValue, PyValue>(bytecode, env);
        }, py::arg("bytecode"), py::arg("env").none(true) = py::none())
        .def("bindfunc", &StaticVM::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("stack_top", &StaticVM::StackTop, py::return_value_policy::take_ownership)
        // base api
        .def_property("top", &StaticVM::GetTop, &StaticVM::SetTop, py::return_value_policy::reference_internal)
        .def("get_roottable", &StaticVM::getroottable, py::keep_alive<0, 1>())
        .def("collectgarbage", [](StaticVM* vm) -> int {
            return sq_collectgarbage(vm->GetVM());
        })
        ;

    py::class_<GenericVM, std::shared_ptr<GenericVM>>(m, "SQVM")
        .def(py::init<int>(), py::arg("size") = 1024)
        .def_property_readonly("vm", &GenericVM::GetVM)
        .def("dumpstack", &GenericVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def("execute", [](GenericVM* vm, std::string sourcecode, PyValue env) -> PyValue {
            if (std::holds_alternative<py::none>(env)) {
                return vm->ExecuteString<PyValue>(sourcecode);
            }
            return vm->ExecuteString<PyValue, PyValue>(sourcecode, env);
        }, py::arg("sourcecode"), py::arg("env").none(true) = py::none())
        .def("execute_bytecode", [](GenericVM* vm, std::string bytecode, PyValue env) -> PyValue {
            if (std::holds_alternative<py::none>(env)) {
                return vm->ExecuteBytecode<PyValue>(bytecode);
            }
            return vm->ExecuteBytecode<PyValue, PyValue>(bytecode, env);
        }, py::arg("bytecode"), py::arg("env").none(true) = py::none())

        .def("bindfunc", &GenericVM::bindFunc, py::arg("funcname"), py::arg("func"))
        .def("stack_top", &GenericVM::StackTop, py::return_value_policy::take_ownership)
        // base api
        .def_property("top", &GenericVM::GetTop, &GenericVM::SetTop, py::return_value_policy::reference_internal)
        .def("get_roottable", &GenericVM::getroottable, py::keep_alive<0, 1>(), py::return_value_policy::move)
        .def("collectgarbage", [](GenericVM* vm) -> int {
            return sq_collectgarbage(vm->GetVM());
        })
        ;

    m.def("compile", [](std::string& sourcecode, std::string& sourcename) -> py::bytes {
        return compile(sourcecode, sourcename);
    }, py::arg("sourcecode").none(false), py::arg("sourcename") = "__main__");

    m.def("compile_bb", [](std::string& sourcecode, std::string& sourcename) -> py::bytes {
        return compile_bb(sourcecode, sourcename);
    }, py::arg("sourcecode").none(false), py::arg("sourcename") = "__main__");

    m.def("get_static_vm", &get_static_vm);

    m.attr("SQUIRREL_VERSION") = SQUIRREL_VERSION;
}
