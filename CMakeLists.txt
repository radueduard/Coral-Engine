cmake_minimum_required(VERSION 3.29)
cmake_policy(SET CMP0167 NEW)
project(Coral LANGUAGES CXX)

# Find and include the required packages
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found: ${Vulkan_FOUND}")
message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan_VERSION: ${Vulkan_VERSION}")

find_package(glslang REQUIRED)
#find_package(slang REQUIRED)

find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

find_package(Boost COMPONENTS unordered uuid REQUIRED)

set(MONO_INCLUDE_DIRS "C:/Program Files/Mono/include/mono-2.0")
set(MONO_LIB_DIR "C:/Program Files/Mono/lib")

set(SLANG_INCLUDE_DIRS "C:/slang-2025.12-windows-x86_64/lib")
set(SLANG_LIB_DIR "C:/slang-2025.12-windows-x86_64/lib")

# Add the source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")
file(GLOB_RECURSE MODULE "src/*.ixx")

# Add the include directories
include_directories("src")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${MODULE})

# Link the required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE
        glslang::glslang
        glslang::glslang-default-resource-limits
        glslang::SPIRV
        glslang::SPVRemapper)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-cpp)


target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)


target_include_directories(${PROJECT_NAME} PRIVATE ${MONO_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${SLANG_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME}  PRIVATE ${MONO_LIB_DIR})
target_link_libraries(${PROJECT_NAME}  PRIVATE mono-2.0-sgen)

target_link_directories(${PROJECT_NAME}  PRIVATE ${SLANG_LIB_DIR})
target_link_libraries(${PROJECT_NAME}  PRIVATE slang)

# Dear ImGui
set(IMGUI_DIR external/imgui)
add_subdirectory(${IMGUI_DIR} imgui)

target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR}/Include)

# Math module
set(MATH_DIR math)
add_subdirectory(${MATH_DIR} math)
target_link_libraries(${PROJECT_NAME} PRIVATE math)

## imgui-node-editor
#set(IMGUI_NODE_EDITOR_DIR external/imgui-node-editor)
#add_subdirectory(${IMGUI_NODE_EDITOR_DIR} imgui-node-editor)
#
#target_link_libraries(${PROJECT_NAME} PRIVATE imgui_node_editor)
#target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_NODE_EDITOR_DIR}/Include)
