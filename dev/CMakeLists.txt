cmake_minimum_required(VERSION 3.10)
project(CPL VERSION 1.0)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenMP REQUIRED)

set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3")

option(ENABLE_DEBUG_PRINT "打印信息" ON)
if(ENABLE_DEBUG_PRINT)
  add_compile_definitions(DEBUG_PRINT)
endif()

# 添加 src 目录到构建
add_subdirectory(src)

# 指定头文件目录
include_directories(include)

