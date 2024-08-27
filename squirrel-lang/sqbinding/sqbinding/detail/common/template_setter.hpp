#pragma once
#ifndef SQOBJECTPTR_SETTER_TEMPLATE
#define SQOBJECTPTR_SETTER_TEMPLATE                                                                                    \
    template <typename TK, typename TV> void set(TK &key, TV &val) {                                                   \
        VM &vm = holder->GetVM();                                                                                      \
        auto sqkey = GenericCast<SQObjectPtr(TK &)>::cast(vm, key);                                                    \
        auto sqval = GenericCast<SQObjectPtr(TV &)>::cast(vm, val);                                                    \
        set(sqkey, sqval);                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TK, typename TV> void set(TK &&key, TV &&val) {                                                 \
        VM &vm = holder->GetVM();                                                                                      \
        auto sqkey = GenericCast<SQObjectPtr(TK &)>::cast(vm, key);                                                    \
        auto sqval = GenericCast<SQObjectPtr(TV &)>::cast(vm, val);                                                    \
        set(sqkey, sqval);                                                                                             \
    }

#endif
