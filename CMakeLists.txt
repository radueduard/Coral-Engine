cmake_minimum_required(VERSION 3.29)
cmake_policy(SET CMP0167 NEW)
project(Coral LANGUAGES CXX)

# Find and include the required packages
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan 1.4 COMPONENTS glslc SPIRV-Tools REQUIRED)
message(STATUS "Vulkan found: ${Vulkan_FOUND}")
message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan_VERSION: ${Vulkan_VERSION}")

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(Boost COMPONENTS unordered uuid REQUIRED)
find_package(slang CONFIG REQUIRED)

# Add the source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")

# Add the include directories
include_directories("src")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link the required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)
target_link_libraries(${PROJECT_NAME} PRIVATE
        slang::gfx
        slang::slang
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::unordered
        Boost::uuid
        #    Boost::random
)

# Dear ImGui
set(IMGUI_DIR external/imgui)
add_subdirectory(${IMGUI_DIR} imgui)

# Glslang
set(GLSLANG_DIR external/glslang)
add_subdirectory(${GLSLANG_DIR} glslang)

# Spirv-Cross
set(SPIRV_CROSS_DIR external/spirv-cross)
add_subdirectory(${SPIRV_CROSS_DIR} SPIRV-Cross)

# Dear ImGui
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR}/Include)

# Glslang
target_link_libraries(${PROJECT_NAME} PRIVATE
        glslang::glslang
        glslang::glslang-default-resource-limits
        glslang::SPIRV
)

# Spirv-Cross
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-core)
