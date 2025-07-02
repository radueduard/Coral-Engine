//
// Created by radue on 2/17/2025.
//

#pragma once

#include <memory>

template <class T, typename IdType>
class NarryTree {
public:
	NarryTree() = default;
	virtual ~NarryTree() = default;

	[[nodiscard]] T* Parent() const { return m_parent; }
	[[nodiscard]] std::vector<T*> Children() const {
		std::vector<T*> children;
		for (const auto &child : m_children) {
			children.emplace_back(child.get());
		}
		return children;
	}

	void AddChild(std::unique_ptr<T> child) {
		child->m_parent = static_cast<T*>(this);
		m_children.emplace_back(std::move(child));
	}

	void AddChild(T* child) {
		child->m_parent = static_cast<T*>(this);
		m_children.emplace_back(std::unique_ptr<T>(child));
	}

	template <class... Args>
	void AddChild(Args&&... args) {
		auto child = std::make_unique<T>(std::forward<Args>(args)...);
		child->m_parent = static_cast<T*>(this);
		m_children.emplace_back(std::move(child));
	}


	std::unique_ptr<T> Detach() {
		if (m_parent == nullptr) {
			return nullptr;
		}

		const auto it = std::ranges::find_if(m_parent->m_children, [this](const auto &child) {
			return child.get() == this;
		});

		if (it == m_parent->m_children.end()) {
			return nullptr;
		}

		auto result = std::move(*it);
		m_parent->m_children.erase(it);
		result->m_parent = nullptr;
		return result;
	}

	T& FindChild(const IdType& id) {
		for (const auto &child: m_children) {
			if (child->Id() == id) {
				return *child;
			}
			try {
				return child->FindChild(id);
			} catch (const std::runtime_error&) {}
		}
		throw std::runtime_error("Child not found");
	}

	virtual IdType Id() const { return m_id; }

	virtual bool operator==(const T& other) const = 0;
	virtual bool operator!=(const T& other) const = 0;

protected:
	IdType m_id;
	T* m_parent = nullptr;
	std::vector<std::unique_ptr<T>> m_children {};
};
