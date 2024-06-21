#include "container.h"
#include "pyfunction.h"

// PythonNativeCall: wrapper for python function, will not pass squirrel env to python object.
// this function will be used to call python func in SQVM and return result to SQVM
SQInteger sqbinding::python::PythonNativeCall(HSQUIRRELVM vm) {
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


// PythonNativeRawCall: wrapper for python function, will pass squirrel env to python object.
// this function will be used to call python func in SQVM and return result to SQVM
SQInteger sqbinding::python::PythonNativeRawCall(HSQUIRRELVM vm) {
    py::gil_scoped_acquire acquire;
    py::function* func;
    sq_getuserdata(vm, -1, (void**)&func, NULL);

    // TODO: 处理参数
    int nparams = sq_gettop(vm) - 2;
    py::list args;
    // 索引从 1 开始, 且位置 1 是 this(env)
    // rawcall 参数从索引 1 开始
    for (int idx = 1; idx <= 1 + nparams; idx ++) {
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
