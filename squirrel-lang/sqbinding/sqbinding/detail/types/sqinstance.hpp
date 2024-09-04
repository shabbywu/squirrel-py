#pragma once
#include "holder.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqbinding/detail/sqdefinition.hpp"
#include "sqvm.hpp"

namespace sqbinding {
namespace detail {
class Instance : public std::enable_shared_from_this<Instance> {
    using Holder = SQObjectPtrHolder<::SQInstance *>;
        using ErrNotFound = sqbinding::key_error;

  public:
    std::shared_ptr<Holder> holder;

  public:
    Instance(::SQInstance *pInstance, VM vm) : holder(std::make_shared<Holder>(pInstance, vm)) {};

    SQUnsignedInteger getRefCount() {
        return pInstance()->_uiRef;
    }
    ::SQInstance *pInstance() {
        return _instance(holder->GetSQObjectPtr());
    }

  public:
    SQOBJECTPTR_SETTER_TEMPLATE
    void set(SQObjectPtr &sqkey, SQObjectPtr &sqval) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();

        sq_pushobject(*vm, self);
        sq_pushobject(*vm, sqkey);
        sq_pushobject(*vm, sqval);
        sq_newslot(*vm, -3, SQFalse);
        sq_pop(*vm, 1);
    }

  public:
    SQOBJECTPTR_GETTER_TEMPLATE
  protected:
    bool get(SQObjectPtr &key, SQObjectPtr &ret) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();
        if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
            return false;
        }
        return true;
    }

  public:
    // bindFunc to current instance
    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        set(funcname, detail::CreateNativeClosure(std::forward<Func>(func), holder->GetVM()));
    }
};
} // namespace detail
} // namespace sqbinding
