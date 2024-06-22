#pragma once

#include "sqbinding/pybinding/common/cast.h"
#include "definition.h"
#include "sqbinding/detail/types/sqstr.hpp"

namespace sqbinding {
    namespace python {
        class String: public detail::String {
            public:
            String(::SQString* pString, detail::VM vm): detail::String(pString, vm) {};

            std::string __str__() {
                return value();
            }

            std::string __repr__() {
                return "\"" + value() + "\"";
            }

            SQInteger __len__() {
                return pString()->_len;
            }
        };
    }
}
