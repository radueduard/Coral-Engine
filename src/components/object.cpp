//
// Created by radue on 10/23/2024.
//

#include "object.h"

#include <ranges>

#include "imgui.h"

namespace Coral {
    Component::Component(const Object &owner) : m_ownerId(owner.UUID()) {}

    Object &Component::Owner() const {
        return *Object::objects.at(m_ownerId);
    }

    Object::Object(const boost::uuids::uuid uuid, const std::string& name) : m_uuid(uuid), m_name(name) {
        objects[m_uuid] = this;
    }

    Object::~Object() {
        objects.erase(m_uuid);
    }

    std::vector<Component*> Object::Components() const {
        std::vector<Component*> result;
        result.reserve(m_components.size());
        for (const auto &component: m_components | std::views::values) {
            result.push_back(component.get());
        }
        return result;
    }

    std::optional<Object *> Object::Find(const boost::uuids::uuid &id) {
        if (id == m_uuid) {
            return this;
        }
        for (const auto &child: m_children) {
            if (auto result = child->Find(id); result.has_value()) {
                return result;
            }
        }
        return std::nullopt;
    }
}
