//
// Created by radue on 11/5/2024.
//

#pragma once

#include <iostream>

#include "manager.h"
#include "elements/element.h"

namespace GUI {
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        Layer(const Layer&) = delete;
        Layer& operator=(const Layer&) = delete;

        virtual void OnGUIAttach() { std::cout << "Attach Called from Layer" << std::endl; }
        virtual void OnGUIUpdate() {}
        void OnGUIRender() const { m_guiObject->Render(); }
        void OnGUIReset() { OnGUIDetach(); OnGUIAttach(); }
        virtual void OnGUIDetach() { std::cout << "Detach Called from Layer" << std::endl; }

    protected:
        std::unique_ptr<Element> m_guiObject = nullptr;
    };
}
