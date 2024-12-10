find_package(vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

file(GLOB_RECURSE IMGUI_SOURCES Source/*.cpp)
file(GLOB_RECURSE IMGUI_HEADERS Include/*.h)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})
target_include_directories(imgui PUBLIC Source)
target_include_directories(imgui PUBLIC Include)

target_link_libraries(imgui PRIVATE Vulkan::Vulkan)
target_link_libraries(imgui PRIVATE glfw)