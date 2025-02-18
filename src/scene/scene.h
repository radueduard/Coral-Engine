//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "gui/layer.h"
#include "components/object.h"
#include "graphics/objects/texture.h"
#include "gui/container.h"
#include "gui/templates/object.h"


namespace mgv {
    class Scene final : public GUI::Layer {
        friend class CalculateMVP;
    public:
        explicit Scene(const Core::Device& device);
        ~Scene() override = default;

        void OnGUIAttach() override;
        void OnGUIDetach() override {}

        void Update(double deltaTime) const;
        void LateUpdate(double deltaTime) const;


        [[nodiscard]] const std::unique_ptr<Object>& Root() const { return m_root; }

    private:
        const Core::Device& m_device;
        std::unique_ptr<Object> m_root;

        std::unique_ptr<Texture> m_testTexture;

        GUI::Container<GUI::InspectorPanel> m_inspectorPanel;
    };
}
