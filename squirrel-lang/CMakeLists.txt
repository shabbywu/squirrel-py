if(NOT DEFINED Python_EXECUTABLE)
  find_package(
    Python3
    COMPONENTS Development
    REQUIRED)
endif()
find_package(pybind11 CONFIG)

file(GLOB MAIN_SOURCE *.cpp *.h)

# 指定编译参数
add_subdirectory(${squirrel_dir} squirrel)
target_compile_options(squirrel_static PRIVATE -fPIC)
target_compile_options(sqstdlib_static PRIVATE -fPIC)

add_subdirectory(sqbinding)

# pybind11_add_module(_squirrel NO_EXTRAS ${MAIN_SOURCE})
pybind11_add_module(_squirrel ${MAIN_SOURCE})
target_link_libraries(_squirrel PUBLIC sqbinding)
set_property(TARGET _squirrel PROPERTY CXX_STANDARD 20)
if(VERSION_INFO)
  target_compile_definitions(_squirrel PRIVATE VERSION_INFO=${VERSION_INFO})
endif()
