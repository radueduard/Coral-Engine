//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "components/object.h"
#include "core/device.h"
#include "gui/layer.h"

namespace mgv {
    class Scene final : public GUI::Layer {
        friend class CalculateMVP;
    public:
        explicit Scene();

        void Update(double deltaTime) const;

        ~Scene() override = default;

        [[nodiscard]] const std::unique_ptr<Object>& Root() const { return m_root; }
        [[nodiscard]] Object& Camera() const { return *m_camera; }

        void InitUI() override;
        void DrawUI() override;
        void UpdateUI() override;
        void DestroyUI() override;

    private:
        void NodeRender(Object* object);
        Object* m_selectedObject = nullptr;

        std::unique_ptr<Object> m_root;
        std::unique_ptr<Object> m_camera;
    };
}
