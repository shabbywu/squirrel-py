# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("squirrel-lang-pybinding")
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-O2 -fno-strict-aliasing)

set(SQ_DISABLE_INTERPRETER "ON")
set(DISABLE_DYNAMIC "ON")

# 包含子项目
if (SQUIRREL305)
  add_subdirectory(libs/SQUIRREL305)
  set(SQUIRRELLIB SQUIRREL305)
else()
  add_subdirectory(libs/SQUIRREL32)
  set(SQUIRRELLIB SQUIRREL32)
endif()

# 指定编译参数
target_compile_options(squirrel_static PRIVATE -fPIC)
target_compile_options(sqstdlib_static PRIVATE -fPIC)


add_subdirectory(libs/pybind11)
add_subdirectory(squirrel-lang)
