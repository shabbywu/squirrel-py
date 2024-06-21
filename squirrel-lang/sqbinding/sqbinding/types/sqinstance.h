#pragma once

#include "sqbinding/common/format.h"
#include "definition.h"
#include "pydict.hpp"


namespace sqbinding {
    namespace detail {
        class Instance: public std::enable_shared_from_this<Instance> {
            public:
                struct Holder {
                    Holder(::SQInstance* pInstance, HSQUIRRELVM vm) : vm(vm) {
                        instance = pInstance;
                        sq_addref(vm, &instance);
                    }
                    ~Holder(){
                        sq_release(vm, &instance);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr instance;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                Instance (::SQInstance* pInstance, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pInstance, vm)) {};

                SQUnsignedInteger getRefCount() {
                    return pInstance() -> _uiRef;
                }
                ::SQInstance* pInstance() {
                    return _instance(holder->instance);
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
                    SQObjectPtr& self = holder->instance;

                    sq_pushobject(vm, self);
                    sq_pushobject(vm, sqkey);
                    sq_pushobject(vm, sqval);
                    sq_newslot(vm, -3, SQFalse);
                    sq_pop(vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    HSQUIRRELVM& vm = holder->vm;
                    auto sqkey = generic_cast<std::remove_reference_t<TK>, SQObjectPtr>(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = generic_cast<SQObjectPtr, std::remove_reference_t<TV>>(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    HSQUIRRELVM& vm = holder->vm;
                    SQObjectPtr& self = holder->instance;
                    if (!vm->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
        };
    }
    namespace python {
        class Instance: public detail::Instance, std::enable_shared_from_this<Instance>{
            public:
            // link to a existed pInstance in vm stack
            Instance (::SQInstance* pInstance, HSQUIRRELVM vm): detail::Instance(pInstance, vm) {};

            PyValue get(PyValue key);
            // bindFunc to current instance
            void bindFunc(std::string funcname, PyValue func);

            // Python Interface
            PyValue __getitem__(PyValue& key);
            PyValue __setitem__(PyValue key, PyValue val);
            py::list keys();

            std::string __str__() {
                return string_format("OT_INSTANCE: [addr={%p}, ref=%d]", pInstance(), getRefCount());
            }

            std::string __repr__() {
                return "SQInstance(" + __str__() + ")";
            }
        };
    }
}
