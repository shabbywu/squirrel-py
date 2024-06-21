#pragma once

#include "definition.h"
#include "sqiterator.h"
#include "object.h"
#include "sqbinding/common/errors.h"


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

            // template <class T>
            // void set(const string& key, const T& v) {
            //     HSQUIRRELVM& vm = holder->vm;
            //     sq_pushobject(vm, holder->table);
            //     sq_pushstring(vm, key.c_str(), key.length());
            //     detail::push(vm, v);
            //     sq_newslot(vm, -3, SQFalse);
            //     sq_pop(vm, 1);
            // }

            // template <class T>
            // T get(const string& key) {
            //     T r;
            //     if(get<T>(key, r)) {
            //         return r;
            //     }
            //     throw py::key_error(key);
            // }

            // template <class T>
            // bool get(const string& key, T& r) {
            //     HSQUIRRELVM& vm = holder->vm;
            //     sq_pushobject(vm, holder->table);
            //     sq_pushstring(vm_, key.data(), key.length());
            //     if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
            //         return false;
            //     }
            //     r = detail::fetch<T, detail::FetchContext::TableEntry>(vm_, -1);
            //     sq_pop(vm_, 2);
            //     return true;
            // }

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
        class Table : public detail::Table, public std::enable_shared_from_this<Table> {
            public:
            Table(HSQUIRRELVM vm): detail::Table(vm) {}
            Table(::SQTable* pTable, HSQUIRRELVM vm): detail::Table(pTable, vm) {}

            PyValue get(PyValue key);
            void set(SQObjectPtr& key, SQObjectPtr& val);
            PyValue set(PyValue key, PyValue val);

            void bindFunc(std::string funcname, PyValue func);

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
