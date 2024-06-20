#include "definition.h"
#include "container.h"

namespace py = pybind11;

struct stack_guard {
    stack_guard(HSQUIRRELVM v) {
        vm = v;
        top = sq_gettop(vm);
    }
    ~stack_guard() {
        sq_settop(vm, top);
    }
    private:
        HSQUIRRELVM vm;
        SQInteger top;
};


inline
void call_setup_arg(HSQUIRRELVM vm) {}

template <class Arg, class... Args> inline
void call_setup_arg(HSQUIRRELVM vm, Arg head, Args... tail) {
    sq_pushobject(vm, head);
    call_setup_arg(vm, tail...);
}

template <class... Args> inline
void call_setup(HSQUIRRELVM vm, const HSQOBJECT& closure, const HSQOBJECT& table, Args... args) {
    sq_pushobject(vm, closure);
    sq_pushobject(vm, table);
    call_setup_arg(vm, args...);
}


PyValue sqbinding::python::NativeClosure::operator()(py::args args) {
    SQObjectPtr result;
    HSQUIRRELVM vm = holder->vm;
    stack_guard stack_guard(vm);
    sq_pushobject(vm, holder->nativeClosure);

    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
        sq_pushobject(vm, pthis);
    } else {
        sq_pushroottable(vm);
    }

    // push args into stack
    for (auto var_ : args) {
        auto var = pyvalue_tosqobject(std::move(var_.cast<PyValue>()), vm);
        sq_pushobject(vm, std::move(var));
    }

    if (SQ_FAILED(sq_call(vm, args.size() + 1, SQTrue, SQTrue))) {
        const SQChar* sqErr;
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL) {
            throw std::runtime_error("unknown error");
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        throw std::runtime_error(std::string(sqErr));
    } else {
        SQObject ref;
        sq_getstackobj(vm, -1, &ref);
        result = ref;
    }
    auto v = sqobject_topython(result, vm);
    return std::move(v);
}


PyValue sqbinding::python::Closure::operator()(py::args args) {
    SQObjectPtr result;
    HSQUIRRELVM vm = holder->vm;
    stack_guard stack_guard(vm);
    sq_pushobject(vm, holder->closure);
    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
        sq_pushobject(vm, pthis);
    } else {
        sq_pushroottable(vm);
    }

    // push args into stack
    for (auto var_ : args) {
        auto var = pyvalue_tosqobject(std::move(var_.cast<PyValue>()), vm);
        sq_pushobject(vm, std::move(var));
    }

    if (SQ_FAILED(sq_call(vm, args.size() + 1, SQTrue, SQTrue))) {
        const SQChar* sqErr;
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL) {
            throw std::runtime_error("unknown error");
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        throw std::runtime_error(std::string(sqErr));
    } else {
        SQObject ref;
        sq_getstackobj(vm, -1, &ref);
        result = ref;
    }
    auto v = sqobject_topython(result, vm);
    return std::move(v);
}

PyValue sqbinding::python::Closure::get(PyValue key) {
    HSQUIRRELVM vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = holder->closure;
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
            c->bindThis(self);
        }
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
            c->bindThis(self);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}



PyValue sqbinding::python::NativeClosure::get(PyValue key) {
    HSQUIRRELVM vm = holder->vm;
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = {holder->nativeClosure};
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
            c->bindThis(self);
        }
        if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
            auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
            c->bindThis(self);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}
