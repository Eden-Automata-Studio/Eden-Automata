# 自动收集 src 目录下所有的源文件
file(GLOB_RECURSE SOURCES "*.cpp")

# 添加可执行文件
add_executable(SOCSIM ${SOURCES})
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

# 包含头文件目录
target_include_directories(SOCSIM PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SOCSIM PUBLIC OpenMP::OpenMP_CXX ${PNG_LIBRARY})