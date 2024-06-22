#pragma once
#include <squirrel.h>
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqvm.hpp"

namespace sqbinding {
    namespace detail {
        class Array {
            public:
                struct Holder {
                    Holder(::SQArray* pArray, VM vm) : vm(vm) {
                        array = pArray;
                        sq_addref(*vm, &array);
                    }
                    ~Holder(){
                        sq_release(*vm, &array);
                    }
                    VM vm;
                    SQObjectPtr array;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                Array(VM vm): holder(std::make_shared<Holder>(SQArray::Create(_ss(*vm), 4), vm)) {}
                Array(::SQArray* pArray, VM vm): holder(std::make_shared<Holder>(pArray, vm)) {}

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
                    VM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    auto sqval = generic_cast<std::remove_reference_t<TV>, SQObjectPtr>(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    VM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    auto sqval = generic_cast<std::remove_reference_t<TV>, SQObjectPtr>(vm, val);
                    set(sqkey, sqval);
                }

                template <>
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->array;

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_set(*vm, -3);
                    sq_pop(*vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& idx) {
                    TV r;
                    if(get(idx, r)) {
                        return r;
                    }
                    VM& vm = holder->vm;
                    auto sqidx = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, idx);
                    throw sqbinding::index_error(sqobject_to_string(sqidx));
                }

                template <typename TK, typename TV>
                bool get(TK& idx, TV& r) {
                    VM& vm = holder->vm;
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
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->array;
                    if (!(*vm)->Get(self, idx, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
                template <typename Type>
                void append(Type& obj) {
                    VM& vm = holder->vm;
                    auto sqobj = generic_cast<std::remove_reference_t<Type>, SQObjectPtr>(vm, obj);
                    pArray()->Append(std::move(sqobj));
                }

                template <typename Type>
                std::remove_reference_t<Type> pop(Type& obj) {
                    VM& vm = holder->vm;
                    if (pArray()->Size() < 1) {
                        throw sqbinding::index_error("can't pop empty array");
                    }
                    SQObjectPtr sqval = pArray()->Top();
                    pArray()->Pop();
                    return generic_cast<SQObjectPtr, std::remove_reference_t<Type>>(vm, sqval);
                }
        };
    }
}
