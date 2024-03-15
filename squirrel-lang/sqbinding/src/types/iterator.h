#ifndef _SQBINDING_ITERATOR_H_
#define _SQBINDING_ITERATOR_H_

#include "definition.h"
class _SQArray_;
class _SQTable_;


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
        _SQTable_* obj;
        SQInteger idx = 0;
        
        TableIterator(_SQTable_ *obj);
        PyValue __next__();
};

#endif
