#include "sqbinding/detail/vm/vm.hpp"
#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/pybinding/common/cast.h"
#include "sqbinding/detail/vm/printer.hpp"


namespace sqbinding {
    namespace python {
        class GenericVM: public detail::GenericVM {
        public:
            std::shared_ptr<python::Table> roottable;
        public:
            using detail::GenericVM::GenericVM;
            ~GenericVM(){
                #ifdef TRACE_CONTAINER_GC
                std::cout << "GC::Release python::GenericVM: " << GetSQVM() << std::endl;
                #endif
                roottable = nullptr;
            }
        public:
            std::shared_ptr<python::Table>& getroottable() {
                if (roottable == nullptr) {
                    roottable = std::make_shared<python::Table>(_table(GetSQVM()->_roottable), GetVM());
                }
                return roottable;
            }
        };
    }
}
