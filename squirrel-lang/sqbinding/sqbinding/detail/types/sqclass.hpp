#pragma once
#include "holder.hpp"
#include "sqbinding/detail/common/cast_def.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqbinding/detail/sqdefinition.hpp"
#include "sqtable.hpp"
#include "sqvm.hpp"
#include <memory>

namespace sqbinding {
namespace detail {
class Class : public std::enable_shared_from_this<Class> {
    using Holder = SQObjectPtrHolder<::SQClass *>;

  public:
    std::shared_ptr<Holder> holder;
    bool closed;

  public:
    Class(::SQClass *pClass, VM vm, bool closed = true)
        : holder(std::make_shared<Holder>(pClass, vm)), closed(closed) {};

  public:
    void close() {
        closed = true;
    }

    SQUnsignedInteger getRefCount() {
        return pClass()->_uiRef;
    }

    ::SQClass *pClass() {
        return _class(holder->GetSQObjectPtr());
    }

  public:
    SQOBJECTPTR_SETTER_TEMPLATE
    void set(SQObjectPtr &sqkey, SQObjectPtr &sqval) {
        VM &vm = holder->GetVM();
        SQObjectPtr &self = holder->GetSQObjectPtr();

        sq_pushobject(*vm, self);
        sq_pushobject(*vm, sqkey);
        sq_pushobject(*vm, sqval);
        sq_newslot(*vm, -3, SQTrue);
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

  private:
    std::shared_ptr<Table> _delegate;
    std::shared_ptr<Table> getDelegate() {
        if (_delegate == nullptr) {
            _delegate = std::make_shared<Table>(holder->GetVM());
            auto _get = [this](std::string property) -> SQObjectPtr {
                SQObjectPtr v;
                if (!_delegate->get(property + ".fget", v)) {
                    ::sq_getstackobj(holder->GetSQVM(), 1, &v);

                    if (_delegate->get(property, v)) {
                        return v;
                    }
                    throw sqbinding::key_error(property + " does not found.");
                }
                ::sq_pushobject(holder->GetSQVM(), v);
                ::sq_push(holder->GetSQVM(), 1);
                ::sq_call(holder->GetSQVM(), 1, SQTrue, SQTrue);
                ::sq_getstackobj(holder->GetSQVM(), -1, &v);
                return v;
            };
            auto _set = [this](std::string property, SQObjectPtr value) {
                SQObjectPtr setter;
                if (!_delegate->get(property + ".fset", setter)) {
                    _delegate->set(property, value);
                    return;
                }
                ::sq_pushobject(holder->GetSQVM(), setter);
                ::sq_push(holder->GetSQVM(), 1);
                ::sq_pushobject(holder->GetSQVM(), value);
                ::sq_call(holder->GetSQVM(), 2, SQFalse, SQTrue);
            };
            bindFunc("_get", _get);
            bindFunc("_set", _set);
        }
        return _delegate;
    }

  public:
    template <typename TClass, typename P> void defProperty(std::string property, P TClass::*pm) {
        auto fget = detail::NativeClosure<P(TClass *)>::template Create<detail::cpp_function<1>>(
            [=](TClass *self) -> P { return self->*pm; }, holder->GetVM(), detail::cpp_function<1>::caller);
        auto fset = detail::NativeClosure<void(TClass *, P)>::template Create<detail::cpp_function<1>>(
            [=](TClass *self, P value) { self->*pm = value; }, holder->GetVM(), detail::cpp_function<1>::caller);

        getDelegate()->set(property + ".fget", fget);
        getDelegate()->set(property + ".fset", fset);
    }

  public:
    // bindFunc to current class
    template <typename Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        set(funcname, detail::CreateNativeClosure(std::forward<Func>(func), holder->GetVM()));
    }
};
} // namespace detail
} // namespace sqbinding

namespace sqbinding {
namespace detail {
class ClassRegistry;
static std::map<void *, std::shared_ptr<ClassRegistry>> instances;

class ClassRegistry {
  private:
    std::map<size_t, std::shared_ptr<Class>> class_map;

  public:
    ClassRegistry(VM vm) {
    }

  public:
    static std::shared_ptr<ClassRegistry> getInstance(VM vm) {
        auto k = (void *)(*vm);
        auto i = instances.find(k);
        if (i == instances.end()) {
            auto ptr = std::make_shared<ClassRegistry>(vm);
            instances[k] = ptr;
            return ptr;
        }
        return i->second;
    }

  public:
    template <class C> std::shared_ptr<Class> find_class_object() {
        size_t key = typeid(C).hash_code();
        auto i = class_map.find(key);
        if (i == class_map.end()) {
            return nullptr;
        } else {
            auto p = i->second;
            p->close();
            return p;
        }
    }

    template <class C> void register_class(std::shared_ptr<Class> clazz) {
        class_map[typeid(C).hash_code()] = clazz;
    }
};
} // namespace detail
} // namespace sqbinding

namespace sqbinding {
namespace detail {
template <class C, class Base = void> class ClassDef {
    using Holder = SQObjectPtrHolder<::SQClass *>;

  public:
    std::shared_ptr<Class> holder;
    std::string name;

  public:
    static size_t hash() {
        return typeid(C).hash_code();
    }

  public:
    ClassDef(VM vm, const std::string &name) : holder(nullptr), name(name) {
        auto base = ClassRegistry::getInstance(vm)->find_class_object<Base>();
        holder = std::make_shared<Class>(::SQClass::Create(_ss(*vm), base ? base->pClass() : nullptr), vm, false);
        holder->set(std::string("classname"), name);
        ClassRegistry::getInstance(vm)->register_class<C>(holder);
    }

  public:
    ~ClassDef() {
        holder->close();
    }

    ::SQClass *pClass() {
        return holder->pClass();
    }

  public:
    template <typename P> ClassDef<C, Base> &defProperty(std::string property, P C::*pm) {
        holder->defProperty<C, P>(property, pm);
        return *this;
    }

  public:
    // bindFunc to current class
    template <typename Func> ClassDef<C, Base> &bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        if (!holder->closed)
            holder->bindFunc<Func>(funcname, std::forward<Func>(func), withenv);
        return *this;
    }
};
} // namespace detail
} // namespace sqbinding
