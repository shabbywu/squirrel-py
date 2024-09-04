#pragma once
#include "holder.hpp"
#include "sqbinding/detail/cast/cast_any.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqbinding/detail/sqdefinition.hpp"
#include "sqvm.hpp"
#include <iterator>
#include <stdexcept>

namespace sqbinding {
namespace detail {
class ArrayIterator : public std::iterator<std::input_iterator_tag, SQInteger, SQInteger, SQObjectPtr *, SQObjectPtr> {
    using Holder = SQObjectPtrHolder<::SQArray *>;

  public:
    std::shared_ptr<Holder> holder;
    SQInteger idx = 0;

  public:
    ArrayIterator(::SQArray *pArray, detail::VM vm) : holder(std::make_shared<Holder>(pArray, vm)), idx(0) {
    }
    ArrayIterator(std::shared_ptr<Holder> holder, SQInteger idx) : holder(holder), idx(idx) {
    }

  public:
    ::SQArray *pArray() const {
        return _array(holder->GetSQObjectPtr());
    }
    SQInteger size() const {
        return pArray()->Size();
    }

  public:
    ArrayIterator &operator++() {
        idx++;
        return *this;
    }
    ArrayIterator operator++(int) {
        ArrayIterator retval = *this;
        ++(*this);
        return retval;
    }
    bool operator==(ArrayIterator other) const {
        return holder == other.holder && idx == other.idx;
    }
    bool operator!=(ArrayIterator other) const {
        return !(*this == other);
    }
    reference operator*() const {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        SQObjectPtr ret;
        if (!(*vm)->Get(self, idx, ret, false, DONT_FALL_BACK)) {
            throw sqbinding::stop_iteration();
        }
        return ret;
    }

    ArrayIterator begin() {
        return ArrayIterator(holder, 0);
    }
    ArrayIterator end() {
        return ArrayIterator(holder, size());
    }
};

class Array {
    using Holder = SQObjectPtrHolder<::SQArray *>;

  public:
    std::shared_ptr<Holder> holder;

  public:
    Array(detail::VM vm) : holder(std::make_shared<Holder>(SQArray::Create(_ss(*vm), 0), vm)) {
    }
    Array(::SQArray *pArray, detail::VM vm) : holder(std::make_shared<Holder>(pArray, vm)) {
    }

    SQUnsignedInteger getRefCount() {
        return pArray()->_uiRef;
    }

    ::SQArray *pArray() {
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
    void set(SQObjectPtr &sqkey, SQObjectPtr &sqval) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();

        sq_pushobject(*vm, self);
        sq_pushobject(*vm, sqkey);
        sq_pushobject(*vm, sqval);
        sq_set(*vm, -3);
        sq_pop(*vm, 1);
    }

  public:
    SQOBJECTPTR_GETTER_TEMPLATE
    bool get(SQObjectPtr &idx, SQObjectPtr &ret) {
        detail::VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        if (!(*vm)->Get(self, idx, ret, false, DONT_FALL_BACK)) {
            return false;
        }
        return true;
    }

  public:
    template <typename Type> void append(Type &&obj) {
        detail::VM &vm = holder->vm;
        SQObjectPtr sqobj = detail::generic_cast<Type, SQObjectPtr>(vm, std::forward<Type>(obj));
        pArray()->Append(std::move(sqobj));
    }

    template <typename Type> Type pop() {
        detail::VM &vm = holder->vm;
        if (pArray()->Size() < 1) {
            throw sqbinding::index_error("can't pop empty array");
        }
        SQObjectPtr sqval = pArray()->Top();
        pArray()->Pop();
        return detail::generic_cast<SQObjectPtr, Type>(vm, std::forward<SQObjectPtr>(sqval));
    }
};
} // namespace detail
} // namespace sqbinding
