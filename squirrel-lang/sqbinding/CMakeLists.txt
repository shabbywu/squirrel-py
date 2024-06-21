set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(
  GLOB
  SQBINDING_SOURCE
  "sqbinding/*.cpp"
  "sqbinding/types/pybinding/*.cpp"
  "sqbinding/vm/*.cpp"
  "sqbinding/common/*.cpp"
  "sqbinding/module_register/*.cpp")

add_library(sqbinding STATIC ${SQBINDING_SOURCE})
target_link_libraries(sqbinding PUBLIC pybind11::pybind11 squirrel_static
                                       sqstdlib_static)
target_compile_options(sqbinding PUBLIC -fPIC)
target_include_directories(
  sqbinding PUBLIC ${squirrel_dir}/include ${squirrel_dir}/squirrel
                   ${squall_dir} ${CMAKE_CURRENT_LIST_DIR})
target_precompile_headers(sqbinding PUBLIC
                          sqbinding/types/pybinding/definition.h)

set_property(TARGET sqbinding PROPERTY CXX_STANDARD 20)

if(TRACE_CONTAINER_GC)
  target_compile_definitions(sqbinding PUBLIC TRACE_CONTAINER_GC=1)
endif()
