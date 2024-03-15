#include "definition.h"
#include "iterator.h"
#include "must.h"


TableIterator::TableIterator(_SQTable_ *obj) {
    this->obj = obj;
}


PyValue TableIterator::__next__() {
    if (idx < 0) {
        throw py::stop_iteration();
    }
    PyValue key;
    PyValue value;
    bool found;
    while (idx < obj->pTable->_numofnodes) {
        auto n = &obj->pTable->_nodes[idx++];
        if (sq_type(n->key) != tagSQObjectType::OT_NULL) {
            key = sqobject_topython(n->key, obj->vm);
            value = sqobject_topython(n->val, obj->vm);
            found = 1;
            break;
        }
    }
    if (!found) {
        throw py::stop_iteration();
    }
    return py::make_tuple(key, value);
}



PyValue ArrayIterator::__next__() {
    if (idx < 0) {
        throw py::stop_iteration();
    }
    PyValue result;
    try {
        result = obj->__getitem__(idx);
    } catch(const py::index_error& e) {
        throw py::stop_iteration();
    }
    idx++;
    return result;
}