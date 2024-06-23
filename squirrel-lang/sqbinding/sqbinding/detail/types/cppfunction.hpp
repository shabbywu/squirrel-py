#pragma once
#include <functional>
#include <memory>
#include "sqvm.hpp"
#include "sqbinding/detail/common/format.hpp"
#include "sqbinding/detail/common/call_setup.hpp"
#include "sqbinding/detail/common/malloc.hpp"
#include "sqbinding/detail/common/type_traits.hpp"
#include "sqbinding/detail/common/cpp_function.hpp"

namespace sqbinding {
    namespace detail {
        class SQCppFunction {
            public:
                struct Holder {
                    Holder(cpp_function& func, VM vm) : vm(vm) {
                        func = func;
                    }
                    ~Holder(){
                    }
                    VM vm;
                    cpp_function func;
                };
            public:
                std::shared_ptr<Holder> holder;
                SQObjectPtr pthis; // 'this' pointer for sq_call

            public:
                SQCppFunction(cpp_function& func, VM vm): holder(std::make_shared<Holder>(func, vm)) {};
                ::SQUserData* pUserData() {
                    return _userdata(holder->userData);
                }
                SQUnsignedInteger getRefCount() {
                    return pUserData() -> _uiRef;
                }
                void bindThis(SQObjectPtr &pthis) {
                    this -> pthis = pthis;
                }

                // caller from cpp
                template <class Return, class... Args>
                Return operator()(Args... args) {
                    // TODO
                }

            public:
                std::string to_string() {
                    return string_format("OT_NATIVECLOSURE: [addr={%p}, ref=%d]", pNativeClosure(), getRefCount());
                }

            static SQUserData* Create(cpp_function& func, detail::VM vm) {
                // new userdata to store py::function
                auto result = detail::make_stack_object<SQCppFunction, cpp_function, detail::VM>(vm, func, vm);
                auto func_container = result.first;
                auto ud = result.second;
                ud->SetDelegate(func_container->_delegate->pTable());
                ud->_typetag = (void*)PythonTypeTags::TYPE_FUNCTION;
                return ud;
            }
        };
    }
}
