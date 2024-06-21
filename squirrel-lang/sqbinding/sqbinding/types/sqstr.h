#pragma once

#include "definition.h"
#include "object.h"


namespace sqbinding {
    namespace detail {
        class String {
            public:
            struct Holder {
                Holder(::SQString* pString, HSQUIRRELVM vm) : vm(vm) {
                    string_ = pString;
                    sq_addref(vm, &string_);
                }
                ~Holder(){
                    sq_release(vm, &string_);
                }
                HSQUIRRELVM vm;
                SQObjectPtr string_;
            };
            String(::SQString* pString, HSQUIRRELVM vm): holder(std::make_shared<Holder>(pString, vm)) {};

            SQUnsignedInteger getRefCount() {
                return pString() -> _uiRef;
            }

            ::SQString* pString() {
                return _string(holder->string_);
            }
            std::string value() {
                return _stringval(holder->string_);
            }

            std::shared_ptr<Holder> holder;
        };
    }

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


namespace sqbinding { namespace detail {
    #ifdef USE__SQString__
    typedef python::String string;
    #else
    typedef std::string string;
    #endif
}}
