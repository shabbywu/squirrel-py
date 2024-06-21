#pragma once

#include "sqbinding/common/format.h"
#include "sqbinding/common/errors.h"
#include "sqbinding/common/cast.h"
#include "sqbinding/common/stack_operation.h"
#include "definition.h"


namespace sqbinding {
    namespace detail {
        class Table: public std::enable_shared_from_this<Table> {
            public:
                struct Holder {
                    Holder(::SQTable* pTable, HSQUIRRELVM vm) : vm(vm) {
                        table = pTable;
                        sq_addref(vm, &table);
                    }
                    ~Holder(){
                        sq_release(vm, &table);
                    }
                    HSQUIRRELVM vm;
                    SQObjectPtr table;
                };

            Table(HSQUIRRELVM vm): holder(std::make_shared<Holder>(SQTable::Create(_ss(vm), 4), vm)) {}
            Table(::SQTable* pTable, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pTable, vm)) {}

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
                SQObjectPtr& self = holder->table;

                sq_pushobject(vm, self);
                sq_pushobject(vm, sqkey);
                sq_pushobject(vm, sqval);
                sq_newslot(vm, -3, SQFalse);
                sq_pop(vm, 1);
            }

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
                SQObjectPtr& self = holder->table;
                if (!vm->Get(self, key, ret, false, DONT_FALL_BACK)) {
                    return false;
                }
                return true;
            }

            // bindFunc to current table
            // template<class Func>
            // void bindFunc(std::string funcname, Func func);

            SQUnsignedInteger getRefCount() {
                return pTable() -> _uiRef;
            }

            ::SQTable* pTable() {
                return _table(holder->table);
            }
            std::shared_ptr<Holder> holder;
        };
    }

    namespace python {
        class Table;
        // iterator
        class TableIterator {
            public:
                Table* obj;
                SQInteger idx = 0;

                TableIterator(Table *obj): obj(obj) {};
                PyValue __next__();
        };

        class Table : public detail::Table, public std::enable_shared_from_this<Table> {
            public:
            Table(HSQUIRRELVM vm): detail::Table(vm) {}
            Table(::SQTable* pTable, HSQUIRRELVM vm): detail::Table(pTable, vm) {}

            void bindFunc(std::string funcname, PyValue func);
            PyValue get(PyValue&);

            TableIterator __iter__() {
                return TableIterator(this);
            }
            SQInteger __len__() {
                return pTable()->CountUsed();
            }
            PyValue __getitem__(PyValue key);
            PyValue __setitem__(PyValue key, PyValue val);
            void __delitem__(PyValue key);
            py::list keys();

            std::string __str__() {
                return string_format("OT_TABLE: [addr={%p}, ref=%d]", pTable(), pTable()->_uiRef);
            }

            std::string __repr__() {
                return "SQTable(" + __str__() + ")";
            }
        };
    }
}
