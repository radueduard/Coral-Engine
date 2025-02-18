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

    Object::Object(std::string name) : m_id(generator()), m_name(std::move(name)) {
        objects[m_id] = this;
    }

    Object::~Object() {
        objects.erase(m_id);
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
        if (id == m_id) {
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
