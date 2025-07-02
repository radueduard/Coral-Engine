//
// Created by radue on 2/8/2025.
//

#pragma once
#include <type_traits>

#include "manager.h"
#include "layer.h"

namespace Coral::Reef {
    template <typename T>
    class Container {
        static_assert(std::is_base_of_v<Layer, T>, "Container can only be used with classes derived from Layer");
    public:
        Container() = default;
		Container(nullptr_t) : m_layer(nullptr) {}

        explicit Container(T *layer) : m_layer(layer) {
            GlobalManager().AddLayer(m_layer);
            static_cast<Layer *>(m_layer)->OnGUIAttach();
        }
        ~Container() {
            if (m_layer == nullptr) {
                return;
            }
            static_cast<Layer *>(m_layer)->OnGUIDetach();
            GlobalManager().RemoveLayer(m_layer);
            delete m_layer;
        }

        Container(const Container&) = delete;
        Container& operator=(const Container&) = delete;

        Container(Container&& other) noexcept {
	        m_layer = other.m_layer;
			other.m_layer = nullptr;
        }
        Container& operator=(Container&& other) noexcept {
			if (this != &other) {
				delete m_layer;
				m_layer = other.m_layer;
				other.m_layer = nullptr;
			}
			return *this;
		}

        T& operator *() const {
            return *m_layer;
        }

        T* operator ->() const {
            return m_layer;
        }

        void reset() {
        	if (m_layer == nullptr) {
        		return;
        	}
        	static_cast<Layer *>(m_layer)->OnGUIDetach();
        	GlobalManager().RemoveLayer(m_layer);
        	delete m_layer;
        	m_layer = nullptr;
        }

    	bool operator==(const nullptr_t) const {
			return m_layer == nullptr;
		}

		bool operator!=(const nullptr_t) const {
			return m_layer != nullptr;
		}
    private:
        T* m_layer = nullptr;
    };

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<Layer, T>>>
    Container<T> MakeContainer(Args&&... args) {
        return Container<T>(new T(std::forward<Args>(args)...));
    }
}
