#include "definition.h"
#include "container.h"
#include "sqtable.h"
#include "sqbinding/common/cast.h"


PyValue sqbinding::python::Instance::get(PyValue key) {
    HSQUIRRELVM& vm = holder->vm;
    SQObjectPtr& self = holder->instance;
    auto v = detail::Instance::get<PyValue, PyValue>(key);
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
    return v;
}
