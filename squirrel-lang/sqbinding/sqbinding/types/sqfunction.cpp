#include "definition.h"
#include "container.h"
#include "sqbinding/common/cast.h"

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



namespace sqbinding {
    namespace detail {
        inline
        void call_setup_arg(HSQUIRRELVM vm) {}

        template <class Arg, class... Args> inline
        void call_setup_arg(HSQUIRRELVM vm, Arg head, Args... tail) {
            generic_stack_push(vm, head);
            call_setup_arg(vm, tail...);
        }

        template <class... Args> inline
        void call_setup(HSQUIRRELVM vm, const HSQOBJECT& closure, const HSQOBJECT& table, Args... args) {
            sq_pushobject(vm, closure);
            sq_pushobject(vm, table);
            call_setup_arg(vm, args...);
        }
    }
}


template <class Return, class... Args>
Return sqbinding::detail::NativeClosure<Return (Args...)>::operator()(Args... args) {
    HSQUIRRELVM vm = holder->vm;
    stack_guard stack_guard(vm);
    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
        call_setup(vm, holder->nativeClosure, pthis, args...);
    } else {
        call_setup(vm, holder->nativeClosure, vm->_roottable, args...);
    }

    if (SQ_FAILED(sq_call(vm, sizeof...(args)+1, SQTrue, SQTrue))) {
        const SQChar* sqErr;
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL) {
            throw std::runtime_error("unknown error");
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        throw std::runtime_error(std::string(sqErr));
    } else {
        return generic_stack_get<Return>(vm, -1);
    }
}

PyValue sqbinding::python::NativeClosure::__call__(py::args args) {
    return this->operator()(args);
}



template <class Return, class... Args>
Return sqbinding::detail::Closure<Return (Args...)>::operator()(Args... args) {
    HSQUIRRELVM vm = holder->vm;
    stack_guard stack_guard(vm);
    if (sq_type(pthis) != tagSQObjectType::OT_NULL) {
        call_setup(vm, holder->closure, pthis, args...);
    } else {
        call_setup(vm, holder->closure, vm->_roottable, args...);
    }

    if (SQ_FAILED(sq_call(vm, sizeof...(args)+1, SQTrue, SQTrue))) {
        const SQChar* sqErr;
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL) {
            throw std::runtime_error("unknown error");
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        throw std::runtime_error(std::string(sqErr));
    } else {
        return generic_stack_get<Return>(vm, -1);
    }
}


PyValue sqbinding::python::Closure::__call__(py::args args) {
    return this->operator()(args);
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
    throw py::key_error(detail::sqobject_to_string(sqkey));
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
    throw py::key_error(detail::sqobject_to_string(sqkey));
}
