//
// Created by radue on 11/5/2024.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <ranges>

#include "elements/dockable.h"
#include "elements/element.h"

namespace Coral::Reef {
    class Layer {
        template <typename T>
        friend class Container;

        friend class Manager;

    public:
        Layer() = default;
        virtual ~Layer() = default;

        Layer(const Layer&) = delete;
        Layer& operator=(const Layer&) = delete;

    protected:
        virtual void OnGUIAttach() {}
        virtual void OnGUIDetach() {}
        virtual void OnGUIUpdate() {}

        Element* Dockable(const std::string& name) {
            return m_dockables[name].get();
        }

        void AddDockable(const std::string& name, Reef::Dockable* dockable) {
            m_dockables[name] = std::unique_ptr<Reef::Dockable>(dockable);
        }

        void RemoveDockable(const std::string& name) {
            m_dockables.erase(name);
        }

    private:
        void OnGUIRender() const {
            for (const auto& object : m_dockables | std::views::values) {
                object->Render();
            }
        }

        std::unordered_map<std::string, std::unique_ptr<Reef::Dockable>> m_dockables {};
    };
}
