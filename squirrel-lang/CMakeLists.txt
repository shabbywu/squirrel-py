include_directories(${CMAKE_SOURCE_DIR}/libs/pybind11/include)


file(GLOB MAIN_SOURCE
    *.cpp
    *.h
)

add_subdirectory(sqbinding)

pybind11_add_module(_squirrel ${MAIN_SOURCE})
target_link_libraries(_squirrel PUBLIC pybind11::pybind11 sqbinding)
set_property(TARGET _squirrel PROPERTY CXX_STANDARD 17)
target_compile_options(_squirrel PUBLIC -fPIC)
target_compile_definitions(_squirrel PRIVATE VERSION_INFO=${VERSION_INFO})
