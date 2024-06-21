#pragma once

#include "sqbinding/detail/common/format.hpp"
#include "definition.h"
#include "pydict.hpp"
#include "sqbinding/detail/types/sqinstance.hpp"


namespace sqbinding {
    namespace python {
        class Instance: public detail::Instance, std::enable_shared_from_this<Instance>{
            public:
            // link to a existed pInstance in vm stack
            Instance (::SQInstance* pInstance, HSQUIRRELVM vm): detail::Instance(pInstance, vm) {};

            PyValue get(PyValue key);
            // bindFunc to current instance
            void bindFunc(std::string funcname, PyValue func) {
                set(PyValue(funcname), PyValue(func));
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
                HSQUIRRELVM& vm = holder->vm;
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
