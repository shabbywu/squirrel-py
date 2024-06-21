#ifndef _SQBINDING_ARRAY_H_
#define _SQBINDING_ARRAY_H_

#include "definition.h"
#include "sqiterator.h"
#include "object.h"


namespace sqbinding {
    namespace detail {
        class Array {
            public:
                struct Holder {
                    Holder(::SQArray* pArray, HSQUIRRELVM vm) : vm(vm) {
                        array = pArray;
                        sq_addref(vm, &array);
                    }
                    ~Holder(){
                        sq_release(vm, &array);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr array;
                };

            Array(HSQUIRRELVM vm): holder(std::make_shared<Holder>(SQArray::Create(_ss(vm), 4), vm)) {}
            Array(::SQArray* pArray, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pArray, vm)) {}

            SQUnsignedInteger getRefCount() {
                return pArray() -> _uiRef;
            }

            ::SQArray* pArray() {
                return _array(holder->array);
            }
            std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class Array: public detail::Array, public std::enable_shared_from_this<Array> {
            public:
                // create a array in vm stack
                Array(HSQUIRRELVM vm): detail::Array(SQArray::Create(_ss(vm), 4), vm) {}
                Array(SQArray* pArray, HSQUIRRELVM vm): detail::Array(pArray, vm) {}

            // Python Interface
            PyValue __getitem__(int idx);
            PyValue __setitem__(int idx, PyValue val);
            PyValue append(PyValue val);
            PyValue pop();
            ArrayIterator __iter__() {
                return ArrayIterator(this);
            }
            SQInteger __len__() {
                return pArray()->Size();
            }


            std::string __str__() {
                return string_format("OT_ARRAY: [addr={%p}, ref=%d]", pArray(), getRefCount());
            }

            std::string __repr__() {
                return "SQArray(" + __str__() + ")";
            }
        };
    }
}

typedef sqbinding::python::Array _SQArray_;


// class _SQArray_ : public std::enable_shared_from_this<_SQArray_> {
// public:
//     SQObjectPtr handler;
//     SQArray* pArray;
//     HSQUIRRELVM vm;
//     bool releaseOnDestroy = false;

//     // create a array in vm stack
//     _SQArray_ (HSQUIRRELVM vm): vm(vm) {
//         pArray = SQArray::Create(_ss(vm), 4);
//         handler = pArray;
//         sq_addref(vm, &handler);
//     }

//     // link to a existed table in vm stack
//     _SQArray_ (SQArray* pArray, HSQUIRRELVM vm): pArray(pArray), vm(vm), handler(pArray) {
//         sq_addref(vm, &handler);
//     }

//     _SQArray_(const _SQArray_& rhs) {
//         pArray = rhs.pArray;
//         vm = rhs.vm;
//         handler = pArray;
//         sq_addref(vm, &handler);
//     }
//     _SQArray_& operator=(const _SQArray_& rhs) {
//         release();
//         pArray = rhs.pArray;
//         vm = rhs.vm;
//         handler = pArray;
//         sq_addref(vm, &handler);
//         return *this;
//     };

//     ~_SQArray_() {
//         release();
//     }

//     void release() {
//         __check_vmlock(vm)
//         #ifdef TRACE_CONTAINER_GC
//         std::cout << "GC::Release " << __repr__() << " uiRef--=" << pArray -> _uiRef -2 << std::endl;
//         #endif
//         sq_release(vm, &handler);
//         handler.Null();
//     }

//     // Python Interface
//     PyValue __getitem__(int idx);
//     PyValue __setitem__(int idx, PyValue val);
//     PyValue append(PyValue val);
//     PyValue pop();
//     ArrayIterator __iter__() {
//         return ArrayIterator(this);
//     }
//     SQInteger __len__() {
//         return pArray->Size();
//     }


//     std::string __str__() {
//         return string_format("OT_ARRAY: [addr={%p}, ref=%d]", pArray, pArray->_uiRef);
//     }

//     std::string __repr__() {
//         return "SQArray(" + __str__() + ")";
//     }
// };
#endif
