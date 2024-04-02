#include "definition.h"
#include "container.h"

namespace py = pybind11;


SQInteger PythonNativeCall(HSQUIRRELVM vm) {
    py::gil_scoped_acquire acquire;
    py::function* func;
    sq_getuserdata(vm, -1, (void**)&func, NULL);

    // TODO: 处理参数
    int nparams = sq_gettop(vm) - 2;
    py::list args;
    // 索引从 1 开始, 且位置 1 是 this(env)
    // 参数从索引 2 开始
    for (int idx = 2; idx <= 1 + nparams; idx ++) {
        auto arg = stack_get(vm, idx);
        args.append(sqobject_topython(arg, vm));
    }

    PyValue result = (*func)(*args).cast<PyValue>();
    if (std::holds_alternative<py::none>(result)){
        return 0;
    }
    sq_pushobject(vm, pyvalue_tosqobject(result, vm));
    return 1;
}


PyValue _SQNativeClosure_::__call__(py::args args) {
    SQObjectPtr obj(pNativeClosure);
    SQObjectPtr result;
    SQInteger top = sq_gettop(vm);
    sq_pushobject(vm, obj);
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
            sq_pop(vm, 1);
            throw std::runtime_error("unknown error");
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 2);
        sq_settop(vm, top);
        throw std::runtime_error(std::string(sqErr));
    } else {
        SQObject ref;
        sq_getstackobj(vm, -1, &ref);
        result = ref;
    }
    sq_settop(vm, top);
    auto v = sqobject_topython(result, vm);
    return std::move(v);
}


PyValue _SQClosure_::__call__(py::args args) {
    SQObjectPtr obj(pClosure);
    SQObjectPtr result;
    SQInteger top = sq_gettop(vm);
    sq_pushobject(vm, obj);
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
            sq_pop(vm, 1);
            throw std::runtime_error("unknown error");
        }
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 2);
        throw std::runtime_error(std::string(sqErr));
    } else {
        SQObject ref;
        sq_getstackobj(vm, -1, &ref);
        result = ref;
    }
    sq_settop(vm, top);
    auto v = sqobject_topython(result, vm);
    return std::move(v);
}


PyValue _SQNativeClosure_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = {pNativeClosure};
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<_SQClosure_>>(v)) {
            auto& c = std::get<std::shared_ptr<_SQClosure_>>(v);
            c->bindThis(self);
        }
        if (std::holds_alternative<std::shared_ptr<_SQNativeClosure_>>(v)) {
            auto& c = std::get<std::shared_ptr<_SQNativeClosure_>>(v);
            c->bindThis(self);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}


PyValue _SQClosure_::get(PyValue key) {
    SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
    SQObjectPtr sqval;
    SQObjectPtr self = {pClosure};
    if (vm->Get(self, sqkey, sqval, false, DONT_FALL_BACK)) {
        auto v = sqobject_topython(sqval, vm);
        if (std::holds_alternative<std::shared_ptr<_SQClosure_>>(v)) {
            auto& c = std::get<std::shared_ptr<_SQClosure_>>(v);
            c->bindThis(self);
        }
        if (std::holds_alternative<std::shared_ptr<_SQNativeClosure_>>(v)) {
            auto& c = std::get<std::shared_ptr<_SQNativeClosure_>>(v);
            c->bindThis(self);
        }
        return std::move(v);
    }
    throw py::key_error(sqobject_to_string(sqkey));
}
