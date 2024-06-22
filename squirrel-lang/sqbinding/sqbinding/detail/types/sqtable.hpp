#pragma once

#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/cast.hpp"
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqvm.hpp"


namespace sqbinding {
    namespace detail {
        class Table: public std::enable_shared_from_this<Table> {
            public:
                struct Holder {
                    Holder(::SQTable* pTable, VM vm) : vm(vm) {
                        table = pTable;
                        sq_addref(*vm, &table);
                    }
                    ~Holder(){
                        sq_release(*vm, &table);
                    }
                    VM vm;
                    SQObjectPtr table;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                Table(VM vm): holder(std::make_shared<Holder>(SQTable::Create(_ss(*vm), 4), vm)) {}
                Table(::SQTable* pTable, VM vm): holder(std::make_shared<Holder>(pTable, vm)) {}

                SQUnsignedInteger getRefCount() {
                    return pTable() -> _uiRef;
                }

                ::SQTable* pTable() {
                    return _table(holder->table);
                }
            public:
                template <typename TK, typename TV>
                void set(TK& key, TV& val) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::template cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::template cast(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::template cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::template cast(vm, val);
                    set(sqkey, sqval);
                }

                template <>
                void set(SQObjectPtr& sqkey, SQObjectPtr& sqval) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->table;

                    sq_pushobject(*vm, self);
                    sq_pushobject(*vm, sqkey);
                    sq_pushobject(*vm, sqval);
                    sq_newslot(*vm, -3, SQFalse);
                    sq_pop(*vm, 1);
                }
            public:
                template <typename TK, typename TV>
                TV get(TK& key) {
                    TV r;
                    if(get(key, r)) {
                        return r;
                    }
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::template cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::template cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::template cast(vm, ptr);
                    return true;
                }

                template <>
                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->table;
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }

            // bindFunc to current table
            template<class Func>
            void bindFunc(std::string funcname, Func func) {
                set<std::string, Func>(funcname, func);
            }
        };
    }
}
