#pragma once

#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "definition.h"
#include "sqbinding/detail/types/sqarray.hpp"


namespace sqbinding {
    namespace python {
        class ArrayIterator;
        class Array: public detail::Array, public std::enable_shared_from_this<Array> {
            public:
                // create a array in vm stack
                Array(detail::VM vm): detail::Array(SQArray::Create(_ss(*vm), 4), vm) {}
                Array(SQArray* pArray, detail::VM vm): detail::Array(pArray, vm) {}

                // Python Interface
                PyValue __getitem__(int idx) {
                    return get<int, PyValue>(idx);
                }
                PyValue __setitem__(int idx, PyValue val) {
                    set<int, PyValue>(idx, val);
                    return val;
                }
                std::shared_ptr<ArrayIterator> __iter__() {
                    return std::make_shared<ArrayIterator>(this);
                }
                SQInteger __len__() {
                    return pArray()->Size();
                }

                std::string __str__() {
                    return to_string();
                }

                std::string __repr__() {
                    return "SQArray(" + to_string() + ")";
                }
        };

        class ArrayIterator {
        public:
            sqbinding::python::Array* obj;
            SQInteger idx = 0;

            ArrayIterator(sqbinding::python::Array *obj): obj(obj) {};
            PyValue __next__() {
                if (idx < 0) {
                    throw py::stop_iteration();
                }
                PyValue result;
                try {
                    result = obj->__getitem__(idx);
                } catch(const py::index_error& e) {
                    throw py::stop_iteration();
                }
                idx++;
                return result;
            }
        };
    }
}
