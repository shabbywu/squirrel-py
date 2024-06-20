#ifndef _SQBINDING_STRING_H_
#define _SQBINDING_STRING_H_

#include "definition.h"
#include "object.h"


class _SQString_: public _SQObjectPtr_ {
public:

    // link to a existed table in vm stack
    _SQString_ (SQObjectPtr& pstring, HSQUIRRELVM vm, bool releaseOnDestroy = true) : _SQObjectPtr_(pstring, vm, releaseOnDestroy) {
    }

    _SQString_ (SQString* pstring, HSQUIRRELVM vm) : _SQObjectPtr_(vm, false) {
        obj = SQObjectPtr(pstring);
    }


    std::string __str__() {
        return value();
    }

    std::string __repr__() {
        return "\"" + value() + "\"";
    }

    std::string value() {
        return _stringval(obj);
    }

    SQInteger __len__() {
        return _string(obj)->_len;
    }
};
#endif


namespace sqbinding { namespace detail {
    #ifdef USE__SQString__
    typedef _SQString_ string;
    #else
    typedef std::string string;
    #endif
}}
