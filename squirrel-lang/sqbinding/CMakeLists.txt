set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB SQBINDING_SOURCE "src/*.h" "src/*.cpp" "src/types/*.h"
     "src/types/*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/libs/${SQUIRRELLIB}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/${SQUIRRELLIB}/squirrel)
include_directories(${CMAKE_SOURCE_DIR}/libs/pybind11/include)

add_library(sqbinding STATIC ${SQBINDING_SOURCE})
target_link_libraries(sqbinding PUBLIC pybind11::pybind11 squirrel_static
                                       sqstdlib_static)
target_compile_options(sqbinding PUBLIC -fPIC)
set_property(TARGET sqbinding PROPERTY CXX_STANDARD 20)

if(TRACE_CONTAINER_GC)
  target_compile_definitions(sqbinding PUBLIC TRACE_CONTAINER_GC=1)
endif()
