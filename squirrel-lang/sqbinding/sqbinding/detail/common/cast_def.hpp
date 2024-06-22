#pragma once
#include "sqbinding/detail/types/sqvm.hpp"

namespace sqbinding {
    namespace detail {
        template <typename  FuncSignature>
        class GenericCast;

        template <class FromType, class ToType>
        class GenericCast<ToType(FromType)> {
            public:
            static ToType cast(VM vm, FromType);
        };
    }
}
