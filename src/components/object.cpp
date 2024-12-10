//
// Created by radue on 10/23/2024.
//

#include "object.h"

#include <ranges>

#include "imgui.h"

namespace mgv {
    Component::Component(const Object &owner) : m_ownerId(owner.Id()) {}

    Object &Component::Owner() const {
        return *Object::objects.at(m_ownerId);
    }

    boost::uuids::random_generator Object::generator = boost::uuids::random_generator();
    boost::unordered_map<boost::uuids::uuid, Object*> Object::objects;

    Object::Object(std::string name) : m_id(generator()), m_name(std::move(name)) {
        objects[m_id] = this;
    }

    Object::~Object() {
        objects.erase(m_id);
    }

    std::vector<Object*> Object::Children() const {
        std::vector<Object*> result;
        result.reserve(m_children.size());
        for (const auto &child: m_children | std::views::values) {
            result.push_back(child.get());
        }
        return result;
    }

    std::vector<Component*> Object::Components() const {
        std::vector<Component*> result;
        result.reserve(m_components.size());
        for (const auto &component: m_components | std::views::values) {
            result.push_back(component.get());
        }
        return result;
    }

    void Object::AddChild(std::unique_ptr<Object> child) {
        child->m_parent = this;
        m_children[child->m_id] = std::move(child);
    }

    void Object::RemoveChild(const boost::uuids::uuid &id) {
        m_children.erase(id);
    }

    std::optional<Object *> Object::Find(const boost::uuids::uuid &id) {
        if (m_children.contains(id)) {
            return m_children[id].get();
        }
        for (const auto& [id, child] : m_children) {
            if (auto result = child->Find(id); result.has_value()) {
                return result;
            }
        }
        return std::nullopt;
    }
}
