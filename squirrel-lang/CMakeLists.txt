include_directories(${CMAKE_SOURCE_DIR}/libs/${SQUIRRELLIB}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/${SQUIRRELLIB}/squirrel)
include_directories(${CMAKE_SOURCE_DIR}/libs/pybind11/include)


file(GLOB MAIN_SOURCE
    *.cpp
    *.h
)

add_subdirectory(sqbinding)
pybind11_add_module(_squirrel ${MAIN_SOURCE})

target_link_libraries(_squirrel PRIVATE pybind11::pybind11 sqbinding)
target_compile_definitions(_squirrel PRIVATE VERSION_INFO=${VERSION_INFO})