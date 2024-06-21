#pragma once

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
}


namespace sqbinding { namespace detail {
    #ifdef USE__SQString__
    typedef python::String string;
    #else
    typedef std::string string;
    #endif
}}
