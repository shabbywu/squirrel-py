file(GLOB MAIN_SOURCE *.cpp *.h)

add_subdirectory(sqbinding)

# pybind11_add_module(_squirrel NO_EXTRAS ${MAIN_SOURCE})
pybind11_add_module(_squirrel ${MAIN_SOURCE})
target_link_libraries(_squirrel PUBLIC sqbinding)
set_property(TARGET _squirrel PROPERTY CXX_STANDARD 17)
target_compile_definitions(_squirrel PRIVATE VERSION_INFO=${VERSION_INFO})
target_include_directories(
  _squirrel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../libs/pybind11/include)
