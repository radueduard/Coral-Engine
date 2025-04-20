cmake_minimum_required(VERSION 3.29)
project(Coral)

# Find and include the required packages
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan COMPONENTS glslang shaderc_combined SPIRV-Tools REQUIRED)
message(STATUS "Vulkan found: ${Vulkan_FOUND}")
message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan_VERSION: ${Vulkan_VERSION}")

find_package(glslang REQUIRED)

find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp REQUIRED)
find_package(nlohmann_json REQUIRED)

find_package(Boost COMPONENTS REQUIRED)

set(MONO_INCLUDE_DIRS "C:/Program Files/Mono/include/mono-2.0")
set(MONO_LIB_DIR "C:/Program Files/Mono/lib")

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
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)


target_include_directories(${PROJECT_NAME} PRIVATE ${MONO_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME}  PRIVATE ${MONO_LIB_DIR})
target_link_libraries(${PROJECT_NAME}  PRIVATE mono-2.0-sgen)

# imgui
set(IMGUI_DIR external/imgui)
add_subdirectory(${IMGUI_DIR} imgui)

target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR}/Include)

## imgui-node-editor
#set(IMGUI_NODE_EDITOR_DIR external/imgui-node-editor)
#add_subdirectory(${IMGUI_NODE_EDITOR_DIR} imgui-node-editor)
#
#target_link_libraries(${PROJECT_NAME} PRIVATE imgui-node-editor)
#target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_NODE_EDITOR_DIR}/Include)

#find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
#
#file(GLOB_RECURSE GLSL_SOURCE_FILES
#        "${PROJECT_SOURCE_DIR}/shaders/*.frag"
#        "${PROJECT_SOURCE_DIR}/shaders/*.vert"
#        "${PROJECT_SOURCE_DIR}/shaders/*.geom"
#        "${PROJECT_SOURCE_DIR}/shaders/*.tesc"
#        "${PROJECT_SOURCE_DIR}/shaders/*.tese"
#        "${PROJECT_SOURCE_DIR}/shaders/*.comp"
#        "${PROJECT_SOURCE_DIR}/shaders/*.mesh"
#)
#
#foreach(GLSL ${GLSL_SOURCE_FILES})
#    message(STATUS "BUILDING SHADER")
#    get_filename_component(FILE_NAME ${GLSL} NAME)
#    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/compiled/${FILE_NAME}.spv")
#    message(STATUS ${GLSL})
#    add_custom_command(
#            OUTPUT ${SPIRV}
#            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} --target-env spirv1.6
#            DEPENDS ${GLSL})
#    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
#endforeach(GLSL)
#
#add_custom_target(
#        Shaders
#        DEPENDS ${SPIRV_BINARY_FILES}
#)