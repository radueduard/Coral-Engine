//
// Created by radue on 10/23/2024.
//

#pragma once

#include <memory>
#include <ranges>
#include <typeindex>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/unordered_map.hpp>

#include "gui/layer.h"

namespace mgv {
    struct Transform {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        Transform() : position(0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f) {}

        [[nodiscard]] glm::mat4 Matrix() const {
            const auto rotation = glm::quat(glm::radians(this->rotation));
            return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
        }
    };

    class Component : public GUI::Layer {
        friend class Object;
    public:
        explicit Component(const Object& owner);

        ~Component() override = default;
        virtual void Update(double deltaTime) = 0;

        [[nodiscard]] Object& Owner() const;

        void InitUI() override;
        void UpdateUI() override;
        void DrawUI() override;
        void DestroyUI() override;

    protected:
        boost::uuids::uuid m_ownerId;
    };

    class Object : public Transform {
        friend class Component;

        static boost::uuids::random_generator generator;
        static boost::unordered_map<boost::uuids::uuid, Object*> objects;
    public:
        explicit Object(std::string name = "root");
        ~Object();

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        [[nodiscard]] const boost::uuids::uuid &Id() const { return m_id; }
        [[nodiscard]] const std::string &Name() const { return m_name; }
        [[nodiscard]] Object* Parent() const { return m_parent; }
        [[nodiscard]] std::vector<Object*> Children() const;
        [[nodiscard]] std::vector<Component*> Components() const;
        [[nodiscard]] bool Moved() const { return m_moved; }
        uint32_t& Index() { return m_indexInBuffer; }

        void AddChild(std::unique_ptr<Object> child);
        void RemoveChild(const boost::uuids::uuid& id);

        template<typename T>
        std::optional<T*> Get() {
            const std::type_index type = typeid(T);
            if (!m_components.contains(type)) {
                return std::nullopt;
            }
            auto component = m_components[type].get();
            return static_cast<T*>(component);
        }

        template<typename T>
        std::optional<T*> GetFromChildren() {
            for (const auto &child: m_children | std::views::values) {
                if (auto result = child->Get<T>(); result.has_value()) {
                    return result;
                }
            }
            return std::nullopt;
        }

        template<typename T, typename... Args>
        T* Add(Args&&... args) {
            const std::type_index type = typeid(T);
            if (m_components.contains(type)) {
                return nullptr;
            }
            m_components[type] = std::make_unique<T>(*this, std::forward<Args>(args)...);
            auto component = static_cast<T*>(m_components[type].get());
            component->InitUI();
            return component;
        }

        template<typename T>
        void Remove() {
            const std::type_index type = typeid(T);
            if (!m_components.contains(type)) {
                return;
            }
            m_components[type]->DestroyUI();
            m_components.erase(type);
        }

        std::optional<Object*> Find(const boost::uuids::uuid& id);

    private:
        boost::uuids::uuid m_id;
        std::string m_name;
        uint32_t m_indexInBuffer = 0;
        bool m_moved = false;

        Object *m_parent = nullptr;
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Object>> m_children;
        boost::unordered_map<std::type_index, std::unique_ptr<mgv::Component>> m_components;


    };
}
