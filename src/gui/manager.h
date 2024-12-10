//
// Created by radue on 11/3/2024.
//

#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

namespace Memory::Descriptor {
    class Pool;
}

namespace mgv {
    class Renderer;
}

namespace GUI {
    class Layer;

    class Manager {
    public:
        static void Init();
        static void Destroy();

        static void Update();
        static void Render(const vk::CommandBuffer& commandBuffer);

        static void AddLayer(Layer* layer);
        static void RemoveLayer(Layer* layer);

        Manager(const Manager &) = delete;
        Manager &operator=(const Manager &) = delete;
    private:
        explicit Manager() = default;
        ~Manager() = default;
        inline static Manager* m_instance;


        static void CreateDescriptorPool();
        static void CreateContext();
        static void DestroyContext();

        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;

        std::vector<Layer*> m_layers;
        std::vector<Layer*> m_layersToAdd;
        std::vector<Layer*> m_layersToRemove;
    };
}
