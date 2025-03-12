//
// Created by radue on 2/8/2025.
//

#pragma once
#include <type_traits>

#include "manager.h"

namespace GUI {
    class Layer;

    template <typename T>
    class Container {
        static_assert(std::is_base_of_v<Layer, T>, "T must derive from Layer");
        friend class Layer;
    public:
        Container() = default;
        explicit Container(T *layer) : m_layer(layer) {
            g_manager->AddLayer(m_layer);
            static_cast<Layer *>(m_layer)->OnGUIAttach();
            for (const auto& [name, builder] : static_cast<Layer *>(m_layer)->m_guiBuilder) {
                static_cast<Layer *>(m_layer)->m_guiObjectReset[name] = false;
                static_cast<Layer *>(m_layer)->m_guiObjects.emplace(name, builder());
            }
        }
        ~Container() {
            if (m_layer == nullptr) {
                return;
            }
            static_cast<Layer *>(m_layer)->OnGUIDetach();
            g_manager->RemoveLayer(m_layer);
            delete m_layer;
        }

        Container(const Container&) = delete;
        Container& operator=(const Container&) = delete;

        Container(Container&& other) noexcept {
            m_layer = other.m_layer;
            other.m_layer = nullptr;
        }

        Container& operator=(Container&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            if (m_layer != nullptr) {
                static_cast<Layer *>(m_layer)->OnGUIDetach();
                RemoveLayer(m_layer);
                delete m_layer;
            }
            m_layer = other.m_layer;
            other.m_layer = nullptr;
            return *this;
        }

        T operator *() {
            return *m_layer;
        }

        T* operator ->() const {
            return m_layer;
        }


    private:
        T* m_layer = nullptr;
    };

    template <typename T, typename... Args>
    Container<T> MakeContainer(Args&&... args) {
        return Container<T>(new T(std::forward<Args>(args)...));
    }
}
