#pragma once

#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/pybinding/common/stack_operation.h"
#include "definition.h"
#include "sqbinding/detail/types/sqtable.hpp"

namespace sqbinding {
    namespace python {
        class TableIterator;
        class Table : public detail::Table, public std::enable_shared_from_this<Table> {
            public:
            Table(HSQUIRRELVM vm): detail::Table(vm) {}
            Table(::SQTable* pTable, HSQUIRRELVM vm): detail::Table(pTable, vm) {}

            void bindFunc(std::string funcname, PyValue func) {
                set(PyValue(funcname), PyValue(func));
            }
            PyValue get(PyValue& key);

            std::shared_ptr<TableIterator> __iter__() {
                return std::make_shared<TableIterator>(this);
            }
            SQInteger __len__() {
                return pTable()->CountUsed();
            }
            PyValue __getitem__(PyValue key) {
                return get(key);
            }
            PyValue __setitem__(PyValue key, PyValue val) {
                set(key, val);
                return val;
            }
            void __delitem__(PyValue key) {
                // TODO: add pop(key)
                HSQUIRRELVM& vm = holder->vm;
                SQObjectPtr sqkey = pyvalue_tosqobject(key, vm);
                pTable()->Remove(sqkey);
            }
            py::list keys() {
                HSQUIRRELVM& vm = holder->vm;
                SQInteger idx = 0;
                py::list keys;
                while (idx < pTable()->_numofnodes) {
                    auto n = &pTable()->_nodes[idx++];
                    if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
                        keys.append(sqobject_topython(n->key, vm));
                    }
                }
                return keys;
            }

            std::string __str__() {
                return string_format("OT_TABLE: [addr={%p}, ref=%d]", pTable(), pTable()->_uiRef);
            }

            std::string __repr__() {
                return "SQTable(" + __str__() + ")";
            }
        };

        // iterator
        class TableIterator {
            public:
                Table* obj;
                SQInteger idx = 0;

                TableIterator(Table *obj): obj(obj) {};
                PyValue __next__() {
                    if (idx < 0) {
                        throw py::stop_iteration();
                    }
                    PyValue key;
                    PyValue value;
                    bool found;
                    HSQUIRRELVM& vm = obj->holder->vm;
                    while (idx < obj->pTable()->_numofnodes) {
                        auto n = &obj->pTable()->_nodes[idx++];
                        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
                            key = sqobject_topython(n->key, vm);
                            value = sqobject_topython(n->val, vm);
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        throw py::stop_iteration();
                    }
                    return py::make_tuple(key, value);
                }
        };
    }
}