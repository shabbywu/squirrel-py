#include "definition.h"
#include "container.h"

namespace py = pybind11;


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
    for(auto iter = args.begin(); iter != args.end(); iter ++) {
        auto var_ = iter->cast<py::object>();
        sq_pushobject(vm, pyvalue_tosqobject(var_, vm));
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
    return sqobject_topython(result, vm);
}


SQInteger PythonNativeCall(HSQUIRRELVM vm) {
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

    // TODO: 处理异常
    py::object result = (*func)(*args);
    if(result.is_none()) {
        return 0;
    }
    sq_pushobject(vm, pyvalue_tosqobject(pyobject_topyvalue(result), vm));
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
    for(auto iter = args.begin(); iter != args.end(); iter ++) {
        auto var_ = iter->cast<py::object>();
        sq_pushobject(vm, pyvalue_tosqobject(var_, vm));
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
    return v;
}
