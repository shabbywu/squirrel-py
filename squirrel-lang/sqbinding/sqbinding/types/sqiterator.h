#ifndef _SQBINDING_ITERATOR_H_
#define _SQBINDING_ITERATOR_H_

#include "definition.h"


class ArrayIterator {
public:
    _SQArray_* obj;
    SQInteger idx = 0;

    ArrayIterator(_SQArray_ *obj){
        this->obj = obj;
    };
    PyValue __next__();
};


class TableIterator {
    public:
        sqbinding::python::Table* obj;
        SQInteger idx = 0;

        TableIterator(sqbinding::python::Table *obj);
        PyValue __next__();
};

#endif
