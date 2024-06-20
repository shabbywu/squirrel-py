#include <map>
#include <iostream>
#include <squirrel.h>


namespace vmlock {
    std::map<uintptr_t, int> _vm_handles;
    void register_vm_handle(HSQUIRRELVM vm) {
        #ifdef TRACE_CONTAINER_GC
        std::cout << "register_vm_handle: " << vm << std::endl;
        #endif
        auto k = reinterpret_cast<uintptr_t>(vm);
        if (!_vm_handles.contains(k)) {
            _vm_handles[k] = 1;
            return;
        }
        _vm_handles[k] = _vm_handles[k] + 1;
    }
    void unregister_vm_handle(HSQUIRRELVM vm) {
        auto k = reinterpret_cast<uintptr_t>(vm);
        if (!_vm_handles.contains(k)) {
            #ifdef TRACE_CONTAINER_GC
            std::cout << "unregister_vm_handle but not found: " << vm << std::endl;
            #endif
            return;
        }
        auto v = _vm_handles[k] - 1;
        if (v > 0) {
            #ifdef TRACE_CONTAINER_GC
            std::cout << "unregister_vm_handle ref--: " << vm << std::endl;
            #endif
             _vm_handles[k] = v;
        } else {
            #ifdef TRACE_CONTAINER_GC
            std::cout << "unregister_vm_handle erase: " << vm << std::endl;
            #endif
            _vm_handles.erase(k);
        }
    }
    bool contain_vm_handle(HSQUIRRELVM vm) {
        auto k = reinterpret_cast<uintptr_t>(vm);
        if (!_vm_handles.contains(k)) {return false;}
        return _vm_handles[k] > 0;
    }
    #ifdef TRACE_CONTAINER_GC
    void print_vm_handles() {
        for(auto iter = _vm_handles.begin(); iter != _vm_handles.end(); iter ++) {
            std::cout << "key: " << iter->first << " value: " << iter->second << std::endl;
        }
    }
    #endif
}
