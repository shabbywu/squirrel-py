#pragma once

#include "definition.h"
#include "sqbinding/types/cppbinding/sqstr.hpp"

namespace sqbinding {
    namespace python {
        class String: public detail::String {
            public:
            String(::SQString* pString, HSQUIRRELVM vm): detail::String(pString, vm) {};

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
