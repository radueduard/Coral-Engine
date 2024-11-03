cmake_minimum_required(VERSION 3.5)
project(MotorGraficVulkan)

# Find and include the required packages
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost COMPONENTS REQUIRED)
find_package(glslang CONFIG REQUIRED)

# Add the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Add the include directories
include_directories("src")

# Add the executable
add_executable(MotorGraficVulkan ${SOURCES} ${HEADERS})

# Link the required libraries
target_link_libraries(MotorGraficVulkan PRIVATE Vulkan::Vulkan)
target_link_libraries(MotorGraficVulkan PRIVATE glfw)
target_link_libraries(MotorGraficVulkan PRIVATE assimp::assimp)
target_link_libraries(MotorGraficVulkan PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(MotorGraficVulkan PRIVATE ${Boost_LIBRARIES})
target_link_libraries(MotorGraficVulkan PRIVATE bcrypt)
target_link_libraries(MotorGraficVulkan PRIVATE
        glslang::glslang
        glslang::glslang-default-resource-limits
        glslang::SPIRV
        glslang::SPVRemapper)

set(Stb_INCLUDE_DIR editorAssets/external/stb)
target_include_directories(MotorGraficVulkan PRIVATE ${Stb_INCLUDE_DIR})

# imgui
set(IMGUI_DIR external/imgui)
add_subdirectory(${IMGUI_DIR})

target_link_libraries(MotorGraficVulkan PRIVATE imgui)
target_include_directories(MotorGraficVulkan PRIVATE ${IMGUI_DIR})

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/shaders/*.geom"
        "${PROJECT_SOURCE_DIR}/shaders/*.tesc"
        "${PROJECT_SOURCE_DIR}/shaders/*.tese"
        "${PROJECT_SOURCE_DIR}/shaders/*.comp"
        "${PROJECT_SOURCE_DIR}/shaders/*.mesh"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "BUILDING SHADER")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/compiled/${FILE_NAME}.spv")
    message(STATUS ${GLSL})
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} --target-env spirv1.6
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)