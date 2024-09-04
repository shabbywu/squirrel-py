#pragma once
#ifndef SQFUNCTION_ClosureBase
#define SQFUNCTION_ClosureBase
namespace sqbinding {
namespace detail {
class ClosureBase {
    virtual void bindThis(SQObjectPtr &pthis) = 0;
    virtual void bindThis(SQObjectPtr &&pthis) = 0;
};

template <typename TV>
static inline void bindThisIfNeed(TV &obj, SQObjectPtr &env,
                                  typename std::enable_if_t<std::is_base_of_v<ClosureBase, TV>> * = 0) {
    obj.bindThis(env);
}

template <typename TV>
static inline void bindThisIfNeed(TV &obj, SQObjectPtr &env,
                                  typename std::enable_if_t<!std::is_base_of_v<ClosureBase, TV>> * = 0) {
}
} // namespace detail
} // namespace sqbinding
#endif

#ifndef SQOBJECTPTR_GETTER_TEMPLATE
#define SQOBJECTPTR_GETTER_TEMPLATE                                                                                    \
    template <typename TK, typename TV> TV get(TK &key) {                                                              \
        SQObjectPtr ptr;                                                                                               \
        VM &vm = holder->GetVM();                                                                                      \
        if (get(key, ptr)) {                                                                                           \
            auto v = generic_cast<SQObjectPtr, TV>(vm, std::forward<SQObjectPtr>(ptr));                                \
            bindThisIfNeed(v, holder->GetSQObjectPtr());                                                               \
            return v;                                                                                                  \
        }                                                                                                              \
        SQObjectPtr sqkey = generic_cast<TK, SQObjectPtr>(vm, std::forward<TK>(key));                                  \
        throw ErrNotFound(sqobject_to_string(sqkey) + " does not exists");                                    \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TK, typename TV> TV get(TK &&key) {                                                             \
        SQObjectPtr ptr;                                                                                               \
        VM &vm = holder->GetVM();                                                                                      \
        if (get(key, ptr)) {                                                                                           \
            auto v = generic_cast<SQObjectPtr, TV>(vm, std::forward<SQObjectPtr>(ptr));                                \
            bindThisIfNeed(v, holder->GetSQObjectPtr());                                                               \
            return v;                                                                                                  \
        }                                                                                                              \
        SQObjectPtr sqkey = generic_cast<TK, SQObjectPtr>(vm, std::forward<TK>(key));                                  \
        throw ErrNotFound(sqobject_to_string(sqkey) + " does not exists");                                    \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TK, typename TV> bool get(TK &key, TV &v) {                                                     \
        VM &vm = holder->GetVM();                                                                                      \
        auto sqkey = generic_cast<TK, SQObjectPtr>(vm, std::forward<TK>(key));                                         \
        SQObjectPtr ptr;                                                                                               \
        if (!get(sqkey, ptr)) {                                                                                        \
            return false;                                                                                              \
        }                                                                                                              \
        v = generic_cast<SQObjectPtr, TV>(vm, std::forward<SQObjectPtr>(ptr));                                         \
        bindThisIfNeed(v, holder->GetSQObjectPtr());                                                                   \
        return true;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TK, typename TV> bool get(TK &&key, TV &v) {                                                    \
        VM &vm = holder->GetVM();                                                                                      \
        auto sqkey = generic_cast<TK, SQObjectPtr>(vm, std::forward<TK>(key));                                         \
        SQObjectPtr ptr;                                                                                               \
        if (!get(sqkey, ptr)) {                                                                                        \
            return false;                                                                                              \
        }                                                                                                              \
        v = generic_cast<SQObjectPtr, TV>(vm, std::forward<SQObjectPtr>(ptr));                                         \
        bindThisIfNeed(v, holder->GetSQObjectPtr());                                                                   \
        return true;                                                                                                   \
    }

#endif
