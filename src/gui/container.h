//
// Created by radue on 2/8/2025.
//

#pragma once
#include <type_traits>

namespace GUI {
    class Layer;

    template <typename T>
    class Container {
        static_assert(std::is_base_of_v<Layer, T>, "T must derive from Layer");
    public:
        Container() = default;
        explicit Container(T *layer) : m_layer(layer) {
            AddLayer(m_layer);
            m_layer->OnGUIAttach();
        }
        ~Container() {
            if (m_layer == nullptr) {
                return;
            }
            m_layer->OnGUIDetach();
            RemoveLayer(m_layer);
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
                m_layer->OnGUIDetach();
                RemoveLayer(m_layer);
                delete m_layer;
            }
            m_layer = other.m_layer;
            other.m_layer = nullptr;
            return *this;
        }

    private:
        T* m_layer = nullptr;
    };

    template <typename T, typename... Args>
    Container<T> MakeContainer(Args&&... args) {
        return Container<T>(new T(std::forward<Args>(args)...));
    }
}
