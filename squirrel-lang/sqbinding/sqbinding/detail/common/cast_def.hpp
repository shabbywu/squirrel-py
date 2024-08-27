#pragma once
#include "sqbinding/detail/types/sqvm.hpp"
#include <map>

namespace sqbinding {
namespace detail {
template <typename FuncSignature, class Enable = void> class GenericCast;

template <class FromType, class ToType, class Enable> class GenericCast<ToType(FromType), Enable> {
  public:
    static ToType cast(VM vm, FromType);
};
} // namespace detail
} // namespace sqbinding
