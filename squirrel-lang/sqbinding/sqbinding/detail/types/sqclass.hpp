#pragma once
#include <memory>
#include "sqbinding/detail/common/cast_def.hpp"
#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/template_getter.hpp"
#include "sqbinding/detail/common/template_setter.hpp"
#include "sqvm.hpp"
#include "holder.hpp"


namespace sqbinding {namespace detail {
        class ClassImplBase {
            public:
                virtual ~ClassImplBase() {}

                virtual void close() = 0;
                virtual ::SQClass* pClass() = 0;
        };

        class ClassRegistry;
        static std::map<void *, std::shared_ptr<ClassRegistry>> instances;

        class ClassRegistry
        {
        private:
            std::map<size_t, std::shared_ptr<ClassImplBase>> class_map;

        public:
            ClassRegistry(VM vm) {}
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
            template <class C>
            std::shared_ptr<ClassImplBase> find_class_object() {
                size_t key = typeid(C).hash_code();
                auto i = class_map.find(key);
                if (i == class_map.end())
                    return nullptr;
                else {
                    auto p = i->second;
                    p->close();
                    return p;
                }
            }

            template <class C>
            void register_class(std::shared_ptr<ClassImplBase> clazz) {
                class_map[typeid(C).hash_code()] = clazz;
            }
    };
}}


namespace sqbinding {
    namespace detail {
        class Class: public std::enable_shared_from_this<Class> {
            using Holder = SQObjectPtrHolder<::SQClass*>;
            public:
                std::shared_ptr<Holder> holder;
            public:
                Class (::SQClass* pClass, VM vm): holder(std::make_shared<Holder>(pClass, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pClass() -> _uiRef;
                }

                ::SQClass* pClass() {
                    return _class(holder->GetSQObjectPtr());
                }
            public:
                SQOBJECTPTR_SETTER_TEMPLATE
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_newslot(*vm, -3, SQTrue);
                    sq_pop(*vm, 1);
                }
            public:
                SQOBJECTPTR_GETTER_TEMPLATE
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
            // bindFunc to current class
            template <typename Func>
            void bindFunc(std::string funcname, Func func, bool withenv = false) {
                set<std::string, Func>(funcname, func);
            }
        };


        template<class C>
        class Klass: public ClassImplBase {
            using Holder = SQObjectPtrHolder<::SQClass*>;
            public:
                std::shared_ptr<Holder> holder;
                std::string name;
                bool closed_ = false;

            public:
                static size_t hash() { return typeid(C).hash_code(); }
            public:
                Klass (VM vm, const std::string& name): holder(std::make_shared<Holder>(::SQClass::Create(_ss(*vm), nullptr), vm)), name(name) {
                    ClassRegistry::getInstance(vm)->register_class<C>(std::shared_ptr<ClassImplBase>(this));
                }

                Klass (VM vm, const std::string& name, ::SQClass * base): holder(std::make_shared<Holder>(::SQClass::Create(_ss(*vm), base), vm)), name(name) {
                    ClassRegistry::getInstance(vm)->register_class<C>(std::shared_ptr<ClassImplBase>(this));
                }
            public:
                virtual ~Klass() {

                }
            public:
                SQUnsignedInteger getRefCount() {
                    return pClass() -> _uiRef;
                }

                virtual ::SQClass* pClass() {
                    return _class(holder->GetSQObjectPtr());
                }

                virtual void close() {
                    if (closed_) { return; }
                    set(std::string("classname"), name);
                    closed_ = true;
                }
            public:
                SQOBJECTPTR_SETTER_TEMPLATE
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_newslot(*vm, -3, SQTrue);
                    sq_pop(*vm, 1);
                }
            public:
                SQOBJECTPTR_GETTER_TEMPLATE
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->GetVM();
                    SQObjectPtr& self = holder->GetSQObjectPtr();
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
                // bindFunc to current class
                template<class Func>
                void bindFunc(std::string funcname, Func&& func, bool withenv = false) {
                    set(funcname,
                    detail::NativeClosure<detail::function_signature_t<Func>>::template Create<detail::cpp_function<2>, Func>(
                        std::forward<Func>(func), holder->GetVM(), detail::cpp_function<2>::caller
                    ));
                }

                /// bindFunc a cpp_function from a class method (non-const, no ref-qualifier)
                template <typename Return, typename Class, typename... Args>
                void bindFunc(std::string funcname, Return (Class::*func)(Args...), bool withenv = false) {
                    using Func = Return(Class*, Args...);
                    set(funcname, detail::NativeClosure<Func>::template Create<detail::cpp_function<1>>(
                        func, holder->GetVM(), detail::cpp_function<1>::caller
                    ));
                }
        };
    }
}
