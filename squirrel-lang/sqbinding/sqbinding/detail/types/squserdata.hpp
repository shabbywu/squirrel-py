#pragma once

#include "sqbinding/detail/sqdifinition.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/common/cast_impl.hpp"
#include "sqbinding/detail/common/stack_operation.hpp"
#include "sqvm.hpp"


namespace sqbinding {
    namespace detail {
        template <class T>
        class UserData: public std::enable_shared_from_this<UserData> {
            public:
                struct Holder {
                    Holder(::SQUserData* pUserData, VM vm) : vm(vm) {
                        userdata = pUserData;
                        sq_addref(*vm, &userdata);
                    }
                    ~Holder(){
                        sq_release(*vm, &userdata);
                    }
                    VM vm;
                    ::SQUserData* userdata;
                };
            public:
                std::shared_ptr<Holder> holder;
            public:
                UserData(::SQUserData* pUserData, VM vm): holder(std::make_shared<Holder>(pTable, vm)) {}

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
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::cast(vm, val);
                    set(sqkey, sqval);
                }

                template <typename TK, typename TV>
                void set(TK&& key, TV&& val) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    auto sqval = GenericCast<SQObjectPtr(TV&)>::cast(vm, val);
                    set(sqkey, sqval);
                }

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
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    throw sqbinding::key_error(sqobject_to_string(sqkey));
                }

                template <typename TK, typename TV>
                bool get(TK& key, TV& r) {
                    VM& vm = holder->vm;
                    auto sqkey = GenericCast<SQObjectPtr(TK&)>::cast(vm, key);
                    SQObjectPtr ptr;
                    if (!get(sqkey, ptr)) {
                        return false;
                    }
                    r = GenericCast<TV(SQObjectPtr&)>::cast(vm, ptr);
                    return true;
                }

                bool get(SQObjectPtr& key, SQObjectPtr& ret) {
                    VM& vm = holder->vm;
                    SQObjectPtr& self = holder->table;
                    if (!(*vm)->Get(self, key, ret, false, DONT_FALL_BACK)) {
                        return false;
                    }
                    return true;
                }
            public:
                // bindFunc to current table
                template<class Func>
                void bindFunc(std::string funcname, Func&& func) {
                    set<std::string, Func>(funcname, func);
                }
        };
    }
}
