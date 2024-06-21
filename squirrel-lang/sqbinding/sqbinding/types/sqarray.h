#pragma once

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
