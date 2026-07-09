#include "sqbinding/detail/vm/compiler.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/types/container.h"
#include "sqbinding/pybinding/vm/generic_vm.h"
#include "sqbinding/pybinding/vm/proxy_vm.h"
#include <sqclass.h>
#include <sqobject.h>
#include <squserdata.h>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

typedef sqbinding::python::VMProxy StaticVM;
typedef sqbinding::python::GenericVM GenericVM;

std::shared_ptr<StaticVM> static_vm;
void set_static_vm(HSQUIRRELVM vm) {
    if (static_vm == NULL) {
        static_vm = std::shared_ptr<StaticVM>(new StaticVM(vm), [](StaticVM *static_vm) { delete (static_vm); });
    }
}

std::shared_ptr<StaticVM> get_static_vm() {
    return static_vm;
}

namespace {

bool is_readable_pointer(const void *ptr, std::size_t size) {
    if (ptr == nullptr || size == 0) {
        return false;
    }
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(ptr, &info, sizeof(info)) == 0) {
        return false;
    }
    const auto protect = info.Protect & 0xff;
    if (info.State != MEM_COMMIT || protect == PAGE_NOACCESS || protect == PAGE_EXECUTE) {
        return false;
    }
    const auto begin = reinterpret_cast<std::uintptr_t>(ptr);
    const auto region_begin = reinterpret_cast<std::uintptr_t>(info.BaseAddress);
    const auto region_end = region_begin + info.RegionSize;
    return begin >= region_begin && begin + size <= region_end;
#else
    (void)ptr;
    (void)size;
    return false;
#endif
}

bool is_executable_pointer(const void *ptr) {
    if (ptr == nullptr) {
        return false;
    }
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(ptr, &info, sizeof(info)) == 0) {
        return false;
    }
    const auto protect = info.Protect & 0xff;
    return info.State == MEM_COMMIT
        && (protect == PAGE_EXECUTE || protect == PAGE_EXECUTE_READ || protect == PAGE_EXECUTE_READWRITE
            || protect == PAGE_EXECUTE_WRITECOPY);
#else
    (void)ptr;
    return false;
#endif
}

template <typename T> bool safe_read_value(const void *source, T &out) {
    if (!is_readable_pointer(source, sizeof(T))) {
        return false;
    }
    std::memcpy(&out, source, sizeof(T));
    return true;
}

std::vector<std::string> split_path(const std::string &path) {
    std::vector<std::string> parts;
    std::string current;
    std::istringstream input(path);
    while (std::getline(input, current, '.')) {
        if (!current.empty()) {
            parts.push_back(current);
        }
    }
    return parts;
}

const char *object_type_name(SQObjectType type) {
    switch (type) {
    case OT_NULL: return "null";
    case OT_INTEGER: return "integer";
    case OT_FLOAT: return "float";
    case OT_BOOL: return "bool";
    case OT_STRING: return "string";
    case OT_TABLE: return "table";
    case OT_ARRAY: return "array";
    case OT_USERDATA: return "userdata";
    case OT_CLOSURE: return "closure";
    case OT_NATIVECLOSURE: return "nativeclosure";
    case OT_GENERATOR: return "generator";
    case OT_USERPOINTER: return "userpointer";
    case OT_THREAD: return "thread";
    case OT_FUNCPROTO: return "funcproto";
    case OT_CLASS: return "class";
    case OT_INSTANCE: return "instance";
    case OT_WEAKREF: return "weakref";
    case OT_OUTER: return "outer";
    default: return "unknown";
    }
}

bool can_raw_get_slot(const SQObjectPtr &owner) {
    const auto type = sq_type(owner);
    return type == OT_TABLE || type == OT_CLASS || type == OT_INSTANCE;
}

bool raw_get_slot(HSQUIRRELVM vm, const SQObjectPtr &owner, const std::string &key, SQObjectPtr &out) {
    if (!can_raw_get_slot(owner)) {
        return false;
    }

    const auto top = sq_gettop(vm);
    sq_pushobject(vm, owner);
    sq_pushstring(vm, key.c_str(), -1);
    if (SQ_SUCCEEDED(sq_rawget(vm, -2))) {
        HSQOBJECT raw{};
        sq_getstackobj(vm, -1, &raw);
        out = SQObjectPtr(raw);
        sq_settop(vm, top);
        return true;
    }

    sq_settop(vm, top);
    return false;
}

bool resolve_root_path(HSQUIRRELVM vm, const std::string &path, SQObjectPtr &out, std::string &error) {
    const auto parts = split_path(path);
    if (parts.empty()) {
        error = "empty path";
        return false;
    }

    SQObjectPtr current = vm->_roottable;
    for (const auto &part : parts) {
        SQObjectPtr next;
        if (!raw_get_slot(vm, current, part, next)) {
            error = "missing raw slot: " + part;
            return false;
        }
        current = next;
    }

    out = current;
    return true;
}

bool value_as_native_pointer(const SQObjectPtr &value, std::uintptr_t &pointer) {
    pointer = 0;
    if (sq_type(value) == OT_USERPOINTER) {
        pointer = reinterpret_cast<std::uintptr_t>(_userpointer(value));
        return pointer != 0;
    }
    if (sq_type(value) == OT_USERDATA) {
        std::uint32_t first = 0;
        if (safe_read_value(_userdataval(value), first)) {
            pointer = first;
            return pointer != 0;
        }
    }
    return false;
}

py::dict resolve_native_pointer_from_object(HSQUIRRELVM vm, const SQObjectPtr &obj) {
    py::dict out;
    std::uintptr_t pointer = 0;
    std::string source;
    std::string error;

    SQObjectPtr cpp_instance;
    if (raw_get_slot(vm, obj, "CppInstance", cpp_instance) && value_as_native_pointer(cpp_instance, pointer)) {
        source = "CppInstance";
    } else {
        SQObjectPtr current = obj;
        for (int depth = 0; depth < 8 && is_delegable(current); ++depth) {
            auto *delegable = _delegable(current);
            if (delegable == nullptr || delegable->_delegate == nullptr) {
                break;
            }
            current = SQObjectPtr(delegable->_delegate);
            if (raw_get_slot(vm, current, "CppInstance", cpp_instance) && value_as_native_pointer(cpp_instance, pointer)) {
                source = "delegate.CppInstance";
                break;
            }
        }
    }

    if (pointer == 0 && sq_type(obj) == OT_INSTANCE) {
        auto *instance = _instance(obj);
        if (instance != nullptr && instance->_userpointer != nullptr) {
            pointer = reinterpret_cast<std::uintptr_t>(instance->_userpointer);
            source = "instance.userpointer";
        }
    }

    if (pointer == 0) {
        error = "unable to resolve native pointer";
    }

    const bool pointer_readable = pointer != 0 && is_readable_pointer(reinterpret_cast<const void *>(pointer), 4);
    std::uintptr_t vtable = 0;
    if (pointer_readable) {
        safe_read_value(reinterpret_cast<const void *>(pointer), vtable);
    } else if (pointer != 0) {
        error = "native pointer is not readable object memory";
    }
    const bool vtable_readable = vtable != 0 && is_readable_pointer(reinterpret_cast<const void *>(vtable), 4);
    std::uintptr_t vtable_first_slot = 0;
    if (vtable_readable) {
        safe_read_value(reinterpret_cast<const void *>(vtable), vtable_first_slot);
    }
    const bool vtable_executable = vtable != 0 && is_executable_pointer(reinterpret_cast<const void *>(vtable));
    const bool vtable_first_slot_executable =
        vtable_first_slot != 0 && is_executable_pointer(reinterpret_cast<const void *>(vtable_first_slot));

    out["ok"] = pointer != 0 && pointer_readable && vtable != 0 && vtable_readable;
    out["source"] = source;
    out["error"] = error;
    out["objectType"] = object_type_name(sq_type(obj));
    out["objectTypeValue"] = static_cast<int>(sq_type(obj));
    out["pointer"] = pointer;
    out["pointerReadable"] = pointer_readable;
    out["vtable"] = vtable;
    out["vtableReadable"] = vtable_readable;
    out["vtableExecutable"] = vtable_executable;
    out["vtableFirstSlot"] = vtable_first_slot;
    out["vtableFirstSlotExecutable"] = vtable_first_slot_executable;
    return out;
}

py::dict resolve_native_pointer_by_path(HSQUIRRELVM vm, const std::string &path) {
    py::dict out;
    out["ok"] = false;
    out["path"] = path;

    SQObjectPtr obj;
    std::string error;
    if (!resolve_root_path(vm, path, obj, error)) {
        out["error"] = error;
        return out;
    }

    out = resolve_native_pointer_from_object(vm, obj);
    out["path"] = path;
    return out;
}

} // namespace

void register_squirrel_vm(py::module_ &m) {
    py::class_<StaticVM, std::shared_ptr<StaticVM>>(m, "StaticVM")
        .def_property_readonly("vm", &StaticVM::GetSQVM)
        .def("vm_addr", [](StaticVM *vm) -> std::uintptr_t { return reinterpret_cast<std::uintptr_t>(vm->GetSQVM()); })
        .def("resolve_native_pointer",
             [](StaticVM *vm, const std::string &path) { return resolve_native_pointer_by_path(vm->GetSQVM(), path); },
             py::arg("path"))
        .def("dumpstack", &StaticVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def(
            "execute",
            [](StaticVM *vm, std::string sourcecode, PyValue env) -> PyValue {
                if (std::holds_alternative<py::none>(env)) {
                    return vm->ExecuteString<PyValue>(sourcecode);
                }
                return vm->ExecuteString<PyValue, PyValue>(sourcecode, env);
            },
            py::arg("sourcecode"), py::arg("env").none(true) = py::none())
        .def(
            "execute_bytecode",
            [](StaticVM *vm, std::string bytecode, PyValue env) -> PyValue {
                if (std::holds_alternative<py::none>(env)) {
                    return vm->ExecuteBytecode<PyValue>(bytecode);
                }
                return vm->ExecuteBytecode<PyValue, PyValue>(bytecode, env);
            },
            py::arg("bytecode"), py::arg("env").none(true) = py::none())
        .def("bindfunc", &StaticVM::bindFunc<PyValue &>, py::arg("funcname"), py::arg("func"),
             py::arg("withenv") = false)
        .def("stack_top", &StaticVM::StackTop, py::return_value_policy::take_ownership)
        // base api
        .def_property("top", &StaticVM::GetTop, &StaticVM::SetTop, py::return_value_policy::reference_internal)
        .def("get_roottable", &StaticVM::getroottable, py::return_value_policy::copy)
        .def("root_table", &StaticVM::getroottable, py::return_value_policy::copy)
        .def("collectgarbage", &StaticVM::CollectGarbage);

    py::class_<GenericVM, std::shared_ptr<GenericVM>>(m, "SQVM")
        .def(py::init<int>(), py::arg("size") = 1024)
        .def_property_readonly("vm", &GenericVM::GetSQVM)
        .def("vm_addr", [](GenericVM *vm) -> std::uintptr_t { return reinterpret_cast<std::uintptr_t>(vm->GetSQVM()); })
        .def("resolve_native_pointer",
             [](GenericVM *vm, const std::string &path) { return resolve_native_pointer_by_path(vm->GetSQVM(), path); },
             py::arg("path"))
        .def("dumpstack", &GenericVM::DumpStack, py::arg("stackbase") = -1, py::arg("dumpall") = false)
        .def(
            "execute",
            [](GenericVM *vm, std::string sourcecode, PyValue env) -> PyValue {
                if (std::holds_alternative<py::none>(env)) {
                    return vm->ExecuteString<PyValue>(sourcecode);
                }
                return vm->ExecuteString<PyValue, PyValue>(sourcecode, env);
            },
            py::arg("sourcecode"), py::arg("env").none(true) = py::none())
        .def(
            "execute_bytecode",
            [](GenericVM *vm, std::string bytecode, PyValue env) -> PyValue {
                if (std::holds_alternative<py::none>(env)) {
                    return vm->ExecuteBytecode<PyValue>(bytecode);
                }
                return vm->ExecuteBytecode<PyValue, PyValue>(bytecode, env);
            },
            py::arg("bytecode"), py::arg("env").none(true) = py::none())

        .def("bindfunc", &GenericVM::bindFunc<PyValue &>, py::arg("funcname"), py::arg("func"),
             py::arg("withenv") = false)
        .def("stack_top", &GenericVM::StackTop, py::return_value_policy::take_ownership)
        // base api
        .def_property("top", &GenericVM::GetTop, &GenericVM::SetTop, py::return_value_policy::reference_internal)
        .def("get_roottable", &GenericVM::getroottable, py::return_value_policy::copy)
        .def("root_table", &GenericVM::getroottable, py::return_value_policy::copy)
        .def("collectgarbage", &GenericVM::CollectGarbage);

    m.def(
        "compile",
        [](std::string &sourcecode, std::string &sourcename) -> py::bytes { return compile(sourcecode, sourcename); },
        py::arg("sourcecode").none(false), py::arg("sourcename") = "__main__");

    m.def(
        "compile_bb",
        [](std::string &sourcecode, std::string &sourcename) -> py::bytes {
            return compile_bb(sourcecode, sourcename);
        },
        py::arg("sourcecode").none(false), py::arg("sourcename") = "__main__");

    m.def("get_static_vm", &get_static_vm);

    m.attr("SQUIRREL_VERSION") = SQUIRREL_VERSION;
}
