//
// Created by radue on 2/17/2025.
//

#pragma once

#include <memory>

template <class T>
class NarryTree {
public:
	NarryTree() = default;
	virtual ~NarryTree() = default;

	[[nodiscard]] T* Parent() const { return m_parent; }
	[[nodiscard]] std::vector<T*> Children() const {
		std::vector<T*> result;
		result.reserve(m_children.size());
		for (const auto &child: m_children) {
			result.push_back(child.get());
		}
		return result;
	}

	void AddChild(std::unique_ptr<T> child) {
		child->m_parent = static_cast<T*>(this);
		m_children.push_back(std::move(child));
	}

	template <class... Args>
	void AddChild(Args&&... args) {
		auto child = std::make_unique<T>(std::forward<Args>(args)...);
		child->m_parent = static_cast<T*>(this);
		m_children.push_back(std::move(child));
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

protected:
	T* m_parent = nullptr;
	std::vector<std::unique_ptr<T>> m_children {};
};
