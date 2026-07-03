if(NOT DEFINED Python_EXECUTABLE)
  find_package(
    Python3
    COMPONENTS Interpreter Development
    REQUIRED)
  set(Python_EXECUTABLE
      "${Python3_EXECUTABLE}"
      CACHE FILEPATH "Python executable")
  set(PYTHON_EXECUTABLE
      "${Python3_EXECUTABLE}"
      CACHE FILEPATH "Python executable")
endif()

find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND AND DEFINED Python_EXECUTABLE)
  execute_process(
    COMMAND
      "${Python_EXECUTABLE}" -c
      "import pybind11; print(pybind11.get_cmake_dir())"
    RESULT_VARIABLE _pybind11_python_result
    OUTPUT_VARIABLE _pybind11_cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
  if(_pybind11_python_result EQUAL 0
     AND EXISTS "${_pybind11_cmake_dir}/pybind11Config.cmake")
    set(pybind11_DIR
        "${_pybind11_cmake_dir}"
        CACHE PATH "pybind11 CMake package directory" FORCE)
    find_package(pybind11 CONFIG QUIET)
  endif()
endif()

if(NOT pybind11_FOUND)
  message(
    FATAL_ERROR
      "pybind11 CMake package was not found. Install vcpkg and use its toolchain, "
      "or install the Python package with 'pip install pybind11'.")
endif()

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
