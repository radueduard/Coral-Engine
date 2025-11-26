cmake_minimum_required(VERSION 4.0)
project(ImNodeFlow)

set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/includes/imgui)
set(GLFW_DIR ${CMAKE_CURRENT_LIST_DIR}/includes/glfw)

file(GLOB_RECURSE ImNodeFlow_sources src/*.cpp src/*.h src/*.inl)

add_library(${PROJECT_NAME} ${ImNodeFlow_sources})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} include ${IMGUI_DIR}/backends)
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

target_link_libraries(${PROJECT_NAME} PRIVATE
        imgui
        Vulkan::Vulkan
        glfw
)
