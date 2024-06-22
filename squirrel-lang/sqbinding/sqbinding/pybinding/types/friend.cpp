#include "definition.h"
#include "container.h"

void sqbinding::python::Class::bind_this_if_need(PyValue& v) {
    SQObjectPtr& self = holder->clazz;
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
}

void sqbinding::python::Closure::bind_this_if_need(PyValue& v) {
    SQObjectPtr& self = holder->closure;
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
}

void sqbinding::python::NativeClosure::bind_this_if_need(PyValue& v) {
    SQObjectPtr& self = holder->nativeClosure;
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
}

void sqbinding::python::Instance::bind_this_if_need(PyValue& v) {
    SQObjectPtr& self = holder->instance;
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
}

void sqbinding::python::Table::bind_this_if_need(PyValue& v) {
    SQObjectPtr& self = holder->table;
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::Closure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::Closure>>(v);
        c->bindThis(self);
    }
    if (std::holds_alternative<std::shared_ptr<sqbinding::python::NativeClosure>>(v)) {
        auto& c = std::get<std::shared_ptr<sqbinding::python::NativeClosure>>(v);
        c->bindThis(self);
    }
}
