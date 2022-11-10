# cmake最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 设置PROJECT_NAME变量
set(PROJECT_NAME VulkanTutorial)
# 设置工程名
project (${PROJECT_NAME})

set (CMAKE_CXX_STANDARD 17)
set (3RD_DIR ${PROJECT_SOURCE_DIR}/3rd)
set(CMAKE_MODULE_PATH ${3RD_DIR})

# vulkan
set (VULKAN_DIR ${3RD_DIR}/vulkan)
set (VULKAN_LIB ${VULKAN_DIR}/Lib)
include_directories (${VULKAN_DIR}/Include)
link_directories(${VULKAN_LIB})

# glfw
set (GLFW_DIR ${3RD_DIR}/glfw)
# 保证glfw dir被编译
add_subdirectory (${GLFW_DIR})
# 添加头文件搜索路径
include_directories (${GLFW_DIR}/include)

# glm
set (GLM_DIR ${3RD_DIR}/glm)
include_directories (${GLM_DIR})

# 查找当前目录下的所有源文件并存入DIR_SRCS变量
aux_source_directory(src DIR_SRCS)
# 添加一个可编译的目标到工程
add_executable (${PROJECT_NAME} ${DIR_SRCS})

file(GLOB VULKAN_LIBS "${VULKAN_LIB}/*")
target_link_libraries (${PROJECT_NAME} glfw ${VULKAN_LIBS})


message(STATUS "VULKAN_LIBS = ${VULKAN_LIBS}")
