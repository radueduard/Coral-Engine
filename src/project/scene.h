//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "gui/layer.h"
#include "components/object.h"
#include "graphics/objects/texture.h"
#include "gui/templates/inspector.h"


namespace Coral {
    class Scene final : public GUI::Layer {
    public:
        explicit Scene();
        ~Scene() override = default;

        void OnGUIAttach() override;

        void Update(double deltaTime) const;
        void LateUpdate(double deltaTime) const;


        [[nodiscard]] const std::unique_ptr<Object>& Root() const { return m_root; }

    private:
        std::unique_ptr<Object> m_root;

        GUI::ObjectInspector m_inspectorTemplate;
        Object* m_selectedObject = nullptr;
    };
}
