#include "sqbinding/detail/common/errors.hpp"
#include "sqbinding/detail/vm/printer.hpp"
#include "sqbinding/detail/vm/vm.hpp"
#include "sqbinding/pybinding/common/cast.h"

namespace sqbinding {
namespace python {
class GenericVM : public detail::GenericVM {
  public:
    std::shared_ptr<python::Table> roottable;

  public:
    using detail::GenericVM::GenericVM;
    ~GenericVM() {
#ifdef TRACE_CONTAINER_GC
        std::cout << "GC::Release python::GenericVM: " << GetSQVM() << std::endl;
#endif
        roottable = nullptr;
    }

  public:
    std::shared_ptr<python::Table> &getroottable() {
        if (roottable == nullptr) {
            roottable = std::make_shared<python::Table>(_table(GetSQVM()->_roottable), GetVM());
        }
        return roottable;
    }
    // FIXME: 让 bindfunc 只支持绑定 python 方法?
    template <class Func> void bindFunc(std::string funcname, Func &&func, bool withenv = false) {
        getroottable()->bindFunc(funcname, std::forward<Func>(func), withenv);
    }
};
} // namespace python
} // namespace sqbinding
