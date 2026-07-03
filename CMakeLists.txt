# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置 并包含子项目。
#
cmake_minimum_required(VERSION 3.13)

# Enable VCPKG if it is available and no toolchain was provided.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE
   AND NOT DEFINED ENV{PYODIDE}
   AND DEFINED ENV{VCPKG_ROOT})
  set(_vcpkg_root "$ENV{VCPKG_ROOT}")
  if(_vcpkg_root MATCHES "^~/(.*)")
    set(_vcpkg_root "$ENV{HOME}/${CMAKE_MATCH_1}")
  endif()
  set(_vcpkg_toolchain "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED ENV{PYODIDE})
  find_program(_vcpkg_executable NAMES vcpkg)
  if(_vcpkg_executable)
    get_filename_component(_vcpkg_executable "${_vcpkg_executable}" REALPATH)
    get_filename_component(_vcpkg_root "${_vcpkg_executable}" DIRECTORY)
    set(_vcpkg_toolchain "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")
    if(NOT EXISTS "${_vcpkg_toolchain}")
      get_filename_component(_vcpkg_root "${_vcpkg_root}" DIRECTORY)
      set(_vcpkg_toolchain "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")
    endif()
  endif()
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE
   AND NOT DEFINED ENV{PYODIDE}
   AND EXISTS "${_vcpkg_toolchain}")
  set(CMAKE_TOOLCHAIN_FILE
      "${_vcpkg_toolchain}"
      CACHE FILEPATH "")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE
       AND NOT DEFINED ENV{PYODIDE}
       AND DEFINED ENV{VCPKG_ROOT})
  if(NOT EXISTS "${_vcpkg_toolchain}")
    message(
      WARNING
        "VCPKG_ROOT is set but vcpkg toolchain was not found: ${_vcpkg_toolchain}")
  endif()
elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "${CMAKE_TOOLCHAIN_FILE}"
      CACHE FILEPATH "")
endif()

# Select the vcpkg manifest feature set before the toolchain reads vcpkg.json.
option(INSTALL_PYTHON "Install Python with vcpkg?" OFF)
if(INSTALL_PYTHON)
  list(APPEND VCPKG_MANIFEST_FEATURES "with-python3")
else()
  list(APPEND VCPKG_MANIFEST_FEATURES "without-python3")
endif()

project("PySquirrel")
set(CMAKE_CXX_STANDARD 17)

# 设置变量
if(SQUIRREL305)
  set(squirrel_dir ${CMAKE_CURRENT_LIST_DIR}/libs/SQUIRREL305)
  set(SQUIRRELLIB SQUIRREL305)
else()
  set(squirrel_dir ${CMAKE_CURRENT_LIST_DIR}/libs/SQUIRREL32)
  set(SQUIRRELLIB SQUIRREL32)
endif()
set(squall_dir ${CMAKE_CURRENT_LIST_DIR}/libs/squall)

# 添加子目录
add_subdirectory(squirrel-lang)
# Enable VCPKG if VCPKG_ROOT defined.
option(BUILD_CPP_TESTCASE "Build cpp test?" OFF)
if(${BUILD_CPP_TESTCASE})
  add_subdirectory(tests_cpp)
endif()
