#pragma once
#include <iterator>
#include <stdexcept>
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqvm.hpp"
#include "holder.hpp"

namespace sqbinding {
    namespace detail {
        class ArrayIterator: public std::iterator<
                                        std::input_iterator_tag,
                                        SQInteger,
                                        SQInteger,
                                        SQObjectPtr*,
                                        SQObjectPtr
                                    > {
            using Holder = SQObjectPtrHolder<::SQArray*>;

            public:
                std::shared_ptr<Holder> holder;
                SQInteger idx = 0;
            public:
                ArrayIterator(::SQArray* pArray, detail::VM vm): holder(std::make_shared<Holder>(pArray, vm)), idx(0) {}
                ArrayIterator(std::shared_ptr<Holder> holder, SQInteger idx): holder(holder), idx(idx) {}
            public:
                ::SQArray* pArray() const {
                    return _array(holder->GetSQObjectPtr());
                }
                SQInteger size() const {
                    return pArray()->Size();
                }
            public:
                ArrayIterator& operator++() { idx ++; return *this; }
                ArrayIterator operator++(int) { ArrayIterator retval = *this; ++(*this); return retval; }
                bool operator==(ArrayIterator other) const {
                    return holder == other.holder && idx == other.idx;
                }
                bool operator!=(ArrayIterator other) const { return !(*this == other); }
                reference operator*() const {
                        VM& vm = holder->GetVM();
                        SQObjectPtr& self = holder->GetSQObjectPtr();
                        SQObjectPtr ret;
                        if (!(*vm)->Get(self, idx, ret, false, DONT_FALL_BACK)) {
                            throw sqbinding::stop_iteration();
                        }
                        return ret;
                }

                ArrayIterator begin() { return ArrayIterator(holder, 0); }
                ArrayIterator end() {
                    return ArrayIterator(holder, size());
                }
        };

        class Array {
            using Holder = SQObjectPtrHolder<::SQArray*>;
            public:
                std::shared_ptr<Holder> holder;
            public:
                Array(VM vm): holder(std::make_shared<Holder>(SQArray::Create(_ss(*vm), 4), vm)) {}
                Array(::SQArray* pArray, VM vm): holder(std::make_shared<Holder>(pArray, vm)) {}

                SQUnsignedInteger getRefCount() {
                    return pArray() -> _uiRef;
                }

                ::SQArray* pArray() {
                    return _array(holder->GetSQObjectPtr());
                }
                std::string to_string() {
                    return string_format("OT_ARRAY: [addr={%p}, ref=%d]", pArray(), getRefCount());
                }
                SQInteger size() {
                    return pArray()->Size();
                }
                ArrayIterator iterator() {
                    return ArrayIterator(holder, 0);
                }

            public:
                SQOBJECTPTR_SETTER_TEMPLATE
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();

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
                    auto sqidx = GenericCast<SQObjectPtr(TK&)>::cast(vm, idx);
                    throw sqbinding::index_error(sqobject_to_string(sqidx));
                }

                template <typename TK, typename TV>
                bool get(TK& idx, TV& r) {
                    VM& vm = holder->vm;
                    auto sqidx = GenericCast<SQObjectPtr(TK&)>::cast(vm, idx);
                    SQObjectPtr ptr;
                    if (!get(sqidx, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& idx, SQObjectPtr& ret) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, idx, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
                template <typename Type>
                void append(Type& obj) {
                    VM& vm = holder->vm;
                    auto sqobj = GenericCast<SQObjectPtr(Type&)>::cast(vm, obj);
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
                    return GenericCast<Type(SQObjectPtr&)>::cast(vm, sqval);
                }
        };
    }
}
