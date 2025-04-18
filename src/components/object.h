//
// Created by radue on 10/23/2024.
//

#pragma once

#include <memory>
#include <typeindex>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/unordered_map.hpp>

#include "gui/templates/template.h"
#include "utils/narryTree.h"

namespace GUI {
    class ObjectInspector;
}

namespace Coral {
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

    class Component {
        friend class Object;
    public:
        explicit Component(const Object& owner);
        virtual ~Component() = default;

        virtual void Update(double deltaTime) = 0;
        virtual void LateUpdate(double deltaTime) {}

        [[nodiscard]] Object& Owner() const;

    protected:
        boost::uuids::uuid m_ownerId;
    };

    class Object final : public Transform, public NarryTree<Object> {
        friend class Component;
        friend class GUI::ObjectInspector;

        static boost::uuids::random_generator generator;
        inline static boost::unordered_map<boost::uuids::uuid, Object*> objects;
    public:
        explicit Object(boost::uuids::uuid uuid = boost::uuids::nil_uuid(), const std::string& name = "Object");
        ~Object() override;

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        [[nodiscard]] const boost::uuids::uuid &UUID() const { return m_uuid; }
        [[nodiscard]] const std::string &Name() const { return m_name; }
        [[nodiscard]] std::vector<Component*> Components() const;
        [[nodiscard]] bool Moved() const { return m_moved; }
        uint32_t& Index() { return m_indexInBuffer; }

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
            for (const auto &child: m_children) {
                if (auto result = child->Get<T>(); result.has_value()) {
                    return result;
                }
            }
            return std::nullopt;
        }

        template<typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
        T* Add(Args&&... args) {
            const std::type_index type = typeid(T);
            if (m_components.contains(type)) {
                return nullptr;
            }
            m_components[type] = std::make_unique<T>(*this, std::forward<Args>(args)...);
            auto component = static_cast<T*>(m_components[type].get());
            return component;
        }

        template<typename T>
        void Remove() {
            const std::type_index type = typeid(T);
            if (!m_components.contains(type)) {
                return;
            }
            m_components.erase(type);
        }

        std::optional<Object*> Find(const boost::uuids::uuid& id);

    private:
        boost::uuids::uuid m_uuid;
        std::string m_name;
        uint32_t m_indexInBuffer = 0;
        bool m_moved = false;

        boost::unordered_map<std::type_index, std::unique_ptr<Coral::Component>> m_components;
    };
}

inline std::string to_string(const Coral::Object& object) {
    return object.Name();
}