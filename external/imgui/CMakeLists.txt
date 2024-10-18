find_package(vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

file(GLOB_RECURSE IMGUI_SOURCES *.cpp)
file(GLOB_RECURSE IMGUI_HEADERS *.h)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_link_libraries(imgui PRIVATE Vulkan::Vulkan)
target_link_libraries(imgui PRIVATE glfw)