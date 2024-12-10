//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "gui/layer.h"

#include "graphics/programs/skyBox.h"
#include "graphics/programs/debugCamera.h"

namespace mgv {
    class Object;
}

namespace mgv {
    class Scene final : public GUI::Layer {
        friend class CalculateMVP;
    public:
        explicit Scene();
        ~Scene() override = default;

        void Init();
        void Update(double deltaTime) const;
        void LateUpdate(double deltaTime) const;


        [[nodiscard]] const std::unique_ptr<Object>& Root() const { return m_root; }

        void OnUIAttach() override {}
        void OnUIUpdate() override {}
        void OnUIRender() override;
        void OnUIReset() override {}
        void OnUIDetach() override {}

    private:
        bool NodeRender(Object* object);
        Object* m_selectedObject = nullptr;

        std::unique_ptr<Object> m_root;

        std::unique_ptr<SkyBox> m_skyBoxProgram;
        std::unique_ptr<::DebugCamera> m_debugCamera;

    };
}
