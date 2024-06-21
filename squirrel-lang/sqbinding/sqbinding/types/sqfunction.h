#pragma once

#include <squirrel.h>
#include "sqbinding/common/format.h"
#include "definition.h"

namespace sqbinding {
    namespace detail {
        template <class T>
        class Closure;

        template <class Return, class... Args>
        class Closure<Return (Args...)> {
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

                Closure(::SQClosure* pClosure, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pClosure, vm)) {};
                Return operator()(Args...);
                ::SQClosure* pClosure() {
                    return _closure(holder->closure);
                }
                std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class Closure: public detail::Closure<PyValue (py::args)> {
            public:
                Closure(::SQClosure* pClosure, HSQUIRRELVM vm): detail::Closure<PyValue (py::args)>(pClosure, vm) {
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
                    return "Closure(" + __str__() + ")";
                }

                SQObjectPtr pthis; // 'this' pointer for sq_call
        };
    }
}

namespace sqbinding {
    namespace detail {
        template <class T>
        class NativeClosure;

        template <class Return, class... Args>
        class NativeClosure<Return (Args...)> {
            public:
                struct Holder {
                    Holder(::SQNativeClosure* pNativeClosure, HSQUIRRELVM vm) : vm(vm) {
                        nativeClosure = pNativeClosure;
                        sq_addref(vm, &nativeClosure);
                    }
                    ~Holder(){
                        sq_release(vm, &nativeClosure);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr nativeClosure;
                };

                NativeClosure(::SQNativeClosure* pNativeClosure, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pNativeClosure, vm)) {};
                Return operator()(Args...);
                ::SQNativeClosure* pNativeClosure() {
                    return _nativeclosure(holder->nativeClosure);
                }
                std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class NativeClosure: public detail::NativeClosure<PyValue (py::args)> {
            public:
                NativeClosure(::SQNativeClosure* pNativeClosure, HSQUIRRELVM vm): detail::NativeClosure<PyValue (py::args)>(pNativeClosure, vm) {
                };

            NativeClosure(std::shared_ptr<py::function> func, HSQUIRRELVM vm, SQFUNCTION caller): detail::NativeClosure<PyValue (py::args)>(::SQNativeClosure::Create(_ss(vm), caller, 1), vm) {
                pNativeClosure()->_nparamscheck = 0;
                SQUserPointer ptr = sq_newuserdata(vm, sizeof(py::function));
                std::memcpy(ptr, func.get(), sizeof(py::function));
                pNativeClosure()->_outervalues[0] = vm->PopGet();
            }

                PyValue operator()(py::args args);

                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }
                PyValue get(PyValue key);
                PyValue __call__(py::args args) {
                    return this->operator()(args);
                }
                std::string __str__() {
                    return string_format("OT_NATIVECLOSURE: [addr={%p}, ref=%d]", pNativeClosure(), pNativeClosure()->_uiRef);
                }
                std::string __repr__() {
                    return "NativeClosure(" + __str__() + ")";
                }

                SQObjectPtr pthis; // 'this' pointer for sq_call
        };
    }
}
