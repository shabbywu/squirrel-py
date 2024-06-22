#pragma once

#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/detail/common/format.hpp"
#include "definition.h"
#include "pydict.hpp"
#include "sqbinding/detail/types/sqinstance.hpp"


namespace sqbinding {
    namespace python {
        class Instance: public detail::Instance, std::enable_shared_from_this<Instance>{
            public:
            // link to a existed pInstance in vm stack
            Instance (::SQInstance* pInstance, detail::VM vm): detail::Instance(pInstance, vm) {};
            public:
            void bind_this_if_need(PyValue& v);
            // Python API
            PyValue get(PyValue key) {
                detail::VM& vm = holder->vm;
                SQObjectPtr& self = holder->instance;
                auto v = detail::Instance::get<PyValue, PyValue>(key);
                bind_this_if_need(v);
                return v;
            }
            // bindFunc to current instance
            void bindFunc(std::string funcname, PyValue func) {
                set(funcname, func);
            }

            // Python Interface
            PyValue __getitem__(PyValue& key) {
                return get(key);
            }
            PyValue __setitem__(PyValue key, PyValue val){
                set<PyValue, PyValue>(key, val);
                return val;
            }
            py::list keys() {
                detail::VM& vm = holder->vm;
                SQInteger idx = 0;
                py::list keys;
                auto table = pInstance()->_class->_members;
                while (idx < table->_numofnodes) {
                    auto n = &table->_nodes[idx++];
                    if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
                        keys.append(sqobject_topython(n->key, vm));
                    }
                }
                return keys;
            }

            std::string __str__() {
                return string_format("OT_INSTANCE: [addr={%p}, ref=%d]", pInstance(), getRefCount());
            }

            std::string __repr__() {
                return "SQInstance(" + __str__() + ")";
            }
        };
    }
}
