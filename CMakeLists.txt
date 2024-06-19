# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置 并包含子项目。
#
cmake_minimum_required(VERSION 3.13)

# Enable VCPKG if VCPKG_ROOT defined.
if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE FILEPATH "")
else()
  message("!!! no vcpkg_root")
endif()

# Enable VCPKG if VCPKG_ROOT defined.
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
