#pragma once

#include "sqbinding/common/errors.h"
#include "sqbinding/common/format.h"
#include "definition.h"


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
            public:
                std::shared_ptr<Holder> holder;
            public:
                Array(HSQUIRRELVM vm): holder(std::make_shared<Holder>(SQArray::Create(_ss(vm), 4), vm)) {}
                Array(::SQArray* pArray, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pArray, vm)) {}

                SQUnsignedInteger getRefCount() {
                    return pArray() -> _uiRef;
                }

                ::SQArray* pArray() {
                    return _array(holder->array);
                }
                std::string to_string() {
                    return string_format("OT_ARRAY: [addr={%p}, ref=%d]", pArray(), getRefCount());
                }

            public:
                template <typename TK, typename TV>
                void set(TK& key, TV& val) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    auto sqval = generic_cast<std::remove_reference_t<TV>, SQObjectPtr>(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    auto sqval = generic_cast<std::remove_reference_t<TV>, SQObjectPtr>(vm, val);
                    set(sqkey, sqval);
                }

                template <>
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->array;

                    sq_pushobject(vm, self);
                    sq_pushobject(vm, sqkey);
                    sq_pushobject(vm, sqval);
                    sq_set(vm, -3);
                    sq_pop(vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& idx) {
                    TV r;
                    if(get(idx, r)) {
                        return r;
                    }
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqidx = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, idx);
                    throw sqbinding::index_error(sqobject_to_string(sqidx));
                }

                template <typename TK, typename TV>
                bool get(TK& idx, TV& r) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqidx = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, idx);
                    SQObjectPtr ptr;
                    if (!get(sqidx, ptr)) {
                        return false;
                    }
                    r = generic_cast<SQObjectPtr, std::remove_reference_t<TV>>(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& idx, SQObjectPtr& ret) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->array;
                    if (!vm->Get(self, idx, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
                template <typename Type>
                void append(Type& obj) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqobj = generic_cast<std::remove_reference_t<Type>, SQObjectPtr>(vm, obj);
                    pArray()->Append(std::move(sqobj));
                }

                template <typename Type>
                std::remove_reference_t<Type> pop(Type& obj) {
                    HSQUIRRELVM& vm = holder->vm;
                    if (pArray()->Size() < 1) {
                        throw sqbinding::index_error("can't pop empty array");
                    }
                    SQObjectPtr sqval = pArray()->Top();
                    pArray()->Pop();
                    return generic_cast<SQObjectPtr, std::remove_reference_t<Type>>(vm, sqval);
                }
        };
    }

    namespace python {
        class ArrayIterator;
        class Array: public detail::Array, public std::enable_shared_from_this<Array> {
            public:
                // create a array in vm stack
                Array(HSQUIRRELVM vm): detail::Array(SQArray::Create(_ss(vm), 4), vm) {}
                Array(SQArray* pArray, HSQUIRRELVM vm): detail::Array(pArray, vm) {}

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
