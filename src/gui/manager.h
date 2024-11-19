//
// Created by radue on 11/3/2024.
//

#pragma once
#include "memory/descriptor/pool.h"

namespace mgv {
    class Renderer;
}

namespace GUI {
    class Layer;

    class Manager {
    public:
        static void Init(const mgv::Renderer &renderer);
        static void Destroy();

        static void Update();
        static void Render(const vk::CommandBuffer& commandBuffer);

        static void AddLayer(Layer* layer);
        static void RemoveLayer(Layer* layer);

    private:
        static Manager* m_instance;

        explicit Manager(const mgv::Renderer &renderer);
        ~Manager();

        Manager(const Manager &) = delete;
        Manager &operator=(const Manager &) = delete;

        void CreateDescriptorPool();
        void CreateContext() const;
        void DestroyContext();

        const mgv::Renderer &m_renderer;
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;

        std::vector<Layer*> m_layers;
    };
}
