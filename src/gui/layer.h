//
// Created by radue on 11/5/2024.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <ranges>

#include "elements/element.h"

namespace GUI {
    class Layer {
        template<class T>
        friend class Container;
        friend class Manager;

    public:
        Layer() = default;
        virtual ~Layer() = default;

        Layer(const Layer&) = delete;
        Layer& operator=(const Layer&) = delete;

    protected:
        void ResetElement(const std::string& name) {
            m_guiObjectReset[name] = true;
        }

        virtual void OnGUIAttach() {}
        virtual void OnGUIDetach() {}
        virtual void OnGUIUpdate() {}
        std::unordered_map<std::string, std::function<Element*()>> m_guiBuilder {};

        Element* GUIObject(const std::string& name) {
            return m_guiObjects[name].get();
        }

    private:
        void OnGUIRender() const {
            for (const auto& object : m_guiObjects | std::views::values) {
                object->Render();
            }
        }

        void OnGUIReset() {
            for (auto& [name, object] : m_guiObjects) {
                if (m_guiObjectReset[name]) {
                    object.reset(m_guiBuilder[name]());
                    m_guiObjectReset[name] = false;
                }
            }
        }

        std::unordered_map<std::string, bool> m_guiObjectReset {};
        std::unordered_map<std::string, std::unique_ptr<Element>> m_guiObjects {};

    };
}
