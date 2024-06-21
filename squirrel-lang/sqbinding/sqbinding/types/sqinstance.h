#pragma once

#include "sqbinding/common/format.h"
#include "definition.h"
#include "pydict.h"


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

        };
    }
    namespace python {
        class Instance: public detail::Instance, std::enable_shared_from_this<Instance>{
            public:
            // link to a existed pInstance in vm stack
            Instance (::SQInstance* pInstance, HSQUIRRELVM vm): detail::Instance(pInstance, vm) {};

            PyValue get(PyValue key);
            PyValue getAttributes(PyValue key);
            PyValue setAttributes(PyValue key, PyValue val);
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
