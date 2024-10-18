#include "core/device.h"
#include "core/runtime.h"
#include "core/window.h"
#include "graphics/program.h"
#include "graphics/renderer.h"

#include "extensions/meshShader.h"

int main()
{
    const Core::Window::Info windowInfo {
        .title = "Motor Grafic Vulkan",
        .extent = vk::Extent2D { 1280, 720 },
        .resizable = true,
        .fullscreen = false
    };

    const Core::Window window(windowInfo);
    const Core::Runtime runtime(window);
    const Core::Device device(runtime.PhysicalDevice());
    Graphics::Renderer renderer(window, device);

    Graphics::Program program(device, renderer.SwapChain().RenderPass(), 0);

    const auto drawFunc = [&](const vk::CommandBuffer& commandBuffer) {
        program.BindPipeline(commandBuffer);
        Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, 1, 1, 1);
    };

    while (!window.ShouldClose()) {
        Core::Window::PollEvents();
        if (renderer.BeginFrame()) {
            renderer.Draw(drawFunc);
            renderer.EndFrame();
        }
    }

    return 0;
}
