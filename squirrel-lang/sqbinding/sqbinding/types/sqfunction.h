#ifndef _SQBINDING_CLOSURE_H_
#define _SQBINDING_CLOSURE_H_

#include <squirrel.h>
#include "definition.h"

namespace sqbinding {
    namespace detail {
        template <class T>
        class SQClosure;

        template <class Return, class... Args>
        class SQClosure<Return (Args...)> {
            public:
                struct Holder {
                    Holder(::SQClosure* pClosure, HSQUIRRELVM vm) : vm(vm) {
                        closure = pClosure;
                        sq_addref(vm, &closure);
                    }
                    ~Holder(){
                        sq_release(vm, &closure);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr closure;
                };

                SQClosure(::SQClosure* pClosure, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pClosure, vm)) {};
                Return operator()(Args...);
                ::SQClosure* pClosure() {
                    return _closure(holder->closure);
                }
                std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class SQClosure: public detail::SQClosure<PyValue (py::args)> {
            public:
                SQClosure(::SQClosure* pClosure, HSQUIRRELVM vm): detail::SQClosure<PyValue (py::args)>(pClosure, vm) {
                };

                PyValue operator()(py::args args);

                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }
                PyValue get(PyValue key);
                PyValue __call__(py::args args) {
                    return this->operator()(args);
                }
                std::string __str__() {
                    return string_format("OT_CLOSURE: [addr={%p}, ref=%d]", pClosure(), pClosure()->_uiRef);
                }
                std::string __repr__() {
                    return "SQClosure(" + __str__() + ")";
                }

                SQObjectPtr pthis; // 'this' pointer for sq_call
        };
    }
}

typedef sqbinding::python::SQClosure _SQClosure_;

class _SQNativeClosure_  {
public:
    SQObjectPtr handler;
    HSQUIRRELVM vm = nullptr;
    SQNativeClosure* pNativeClosure;
    SQObjectPtr pthis; // 'this' pointer for sq_call

    _SQNativeClosure_(std::shared_ptr<py::function> func, HSQUIRRELVM vm, SQFUNCTION caller): vm(vm) {
        pNativeClosure = SQNativeClosure::Create(_ss(vm), caller, 1);
        pNativeClosure->_nparamscheck = 0;
        SQUserPointer ptr = sq_newuserdata(vm, sizeof(py::function));
        std::memcpy(ptr, func.get(), sizeof(py::function));
        pNativeClosure->_outervalues[0] = vm->PopGet();
        handler = pNativeClosure;
        sq_addref(vm, &handler);
    }

    // link to a existed table in vm stack
    _SQNativeClosure_ (SQNativeClosure* pNativeClosure, HSQUIRRELVM vm): pNativeClosure(pNativeClosure), vm(vm), handler(pNativeClosure)  {
        sq_addref(vm, &handler);
    }

    _SQNativeClosure_(const _SQNativeClosure_& rhs) {
        pNativeClosure = rhs.pNativeClosure;
        vm = rhs.vm;
        handler = pNativeClosure;
        sq_addref(vm, &handler);
    }
    _SQNativeClosure_& operator=(const _SQNativeClosure_& rhs) {
        release();
        pNativeClosure = rhs.pNativeClosure;
        vm = rhs.vm;
        handler = pNativeClosure;
        sq_addref(vm, &handler);
        return *this;
    };

    ~_SQNativeClosure_() {
        release();
    }

    void release() {
        __check_vmlock(vm)
        #ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release " << __repr__() << " uiRef--=" << this -> pNativeClosure -> _uiRef -2 << std::endl;
        #endif
        sq_release(vm, &handler);
        handler.Null();
    }

    // Python Interface
    void bindThis(SQObjectPtr &pthis) {
        this -> pthis = pthis;
    }
    PyValue get(PyValue key);
    PyValue __call__(py::args args);
    std::string __str__() {
        return string_format("OT_NATIVECLOSURE: [addr={%p}, ref=%d]", pNativeClosure, pNativeClosure->_uiRef);
    }
    std::string __repr__() {
        return "SQNativeClosure(" + __str__() + ")";
    }
};
#endif
