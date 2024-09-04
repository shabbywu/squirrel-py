#pragma once
#ifndef SQOBJECTPTR_SETTER_TEMPLATE
#define SQOBJECTPTR_SETTER_TEMPLATE                                                                                    \
    template <typename TK, typename TV> void set(TK &key, TV &val) {                                                   \
        detail::VM &vm = holder->GetVM();                                                                              \
        auto sqkey = generic_cast<TK, SQObjectPtr>(vm, std::forward<TK>(key));                                         \
        auto sqval = generic_cast<TV, SQObjectPtr>(vm, std::forward<TV>(val));                                         \
        set(sqkey, sqval);                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TK, typename TV> void set(TK &&key, TV &&val) {                                                 \
        detail::VM &vm = holder->GetVM();                                                                              \
        auto sqkey = generic_cast<TK, SQObjectPtr>(vm, std::forward<TK>(key));                                         \
        auto sqval = generic_cast<TV, SQObjectPtr>(vm, std::forward<TV>(val));                                         \
        set(sqkey, sqval);                                                                                             \
    }

#endif
