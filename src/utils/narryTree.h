//
// Created by radue on 2/17/2025.
//

#pragma once

#include <memory>
#include <vector>
#include <boost/unordered_map.hpp>

template <class T, typename IdType>
class Tree {
public:
	class iterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		iterator() : m_current(nullptr) {}
		explicit iterator(T* root) : m_current(root) {
			for (T* child : root->Children()) {
				m_stack.push_back(child);
			}
			if (!m_stack.empty())
				m_current = m_stack.back();
			else
				m_current = nullptr;
		}

		reference operator*() const { return *m_current; }
		pointer operator->() const { return m_current; }

		iterator& operator++() {
			if (m_stack.empty()) {
				m_current = nullptr;
				return *this;
			}

			m_current = m_stack.back();
			m_stack.pop_back();

			auto children = m_current->Children();
			for (auto it = children.rbegin(); it != children.rend(); ++it) {
				m_stack.push_back(*it);
			}

			if (!m_stack.empty()) {
				m_current = m_stack.back();
			} else {
				m_current = nullptr;
			}

			return *this;
		}

		iterator operator++(int) {
			iterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const iterator& other) const {
			return m_current == other.m_current;
		}

		bool operator!=(const iterator& other) const {
			return !(*this == other);
		}

	private:
		T* m_current;
		std::vector<T*> m_stack;
	};

	iterator begin() { return iterator(static_cast<T*>(this)); }
	iterator end() { return iterator(); }

	using reverse_iterator = std::reverse_iterator<iterator>;
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }

	Tree() = default;
	virtual ~Tree() = default;

	[[nodiscard]]
	T* Parent() const { return m_parent; }

	[[nodiscard]]
	std::vector<T*> Children() const {
		std::vector<T*> children;
		for (const auto &child : m_children) {
			children.emplace_back(child.get());
		}
		return children;
	}

	void AddChild(std::unique_ptr<T> child) {
		child->m_parent = static_cast<T*>(this);
		m_childrenMap.emplace(child->Id(), child.get());
		m_children.emplace_back(std::move(child));
	}

	void AddChild(T* child) {
		child->m_parent = static_cast<T*>(this);
		m_childrenMap.emplace(child->Id(), child);
		m_children.emplace_back(child);
	}

	template <class... Args>
	T& EmplaceChild(Args&&... args) {
		auto child = std::make_unique<T>(std::forward<Args>(args)...);
		child->m_parent = static_cast<T*>(this);
		auto childPtr = child.get();
		m_childrenMap.emplace(childPtr->Id(), childPtr);
		m_children.emplace_back(std::move(child));
		return *childPtr;
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
		m_parent->m_childrenMap.erase(m_id);
		result->m_parent = nullptr;
		return result;
	}

	std::optional<T*> FindChild(const IdType& id) const {
		if (m_childrenMap.contains(id)) {
			return m_childrenMap.at(id);
		}
		return std::nullopt;
	}

	std::optional<T*> FindChildRecursive(const IdType& id) const {
		if (m_childrenMap.contains(id)) {
			return m_childrenMap.at(id);
		}
		for (const auto& child : m_children) {
			auto result = child->FindChildRecursive(id);
			if (result.has_value()) {
				return result;
			}
		}
		return std::nullopt;
	}

	void ClearChildren() { m_children.clear(); }

	[[nodiscard]]
	IdType Id() const { return m_id; }

	virtual bool operator==(const Tree& other) const
	{
		if (m_id != other.m_id) {
			return false;
		}
		if (m_children.size() != other.m_children.size()) {
			return false;
		}
		for (const auto& child : m_children) {
			auto find = other.FindChild(child->Id());
			if (!find.has_value() || !(*child == **find)) {
				return false;
			}
		}
		return true;
	}

	virtual bool operator!=(const Tree& other) const {
		return !(*this == other);
	}

protected:
	IdType m_id;
	T* m_parent = nullptr;
	std::vector<std::unique_ptr<T>> m_children {};
	boost::unordered_map<IdType, T*> m_childrenMap;
};

template<class T, typename IdType>
struct std::formatter<Tree<T, IdType>> : std::formatter<IdType> {
	template<typename FormatContext>
	auto format(const Tree<T, IdType>& tree, FormatContext& ctx) const {
		std::string result;
		formatNode(static_cast<const T*>(&tree), result, 0);
		return std::formatter<std::string>::format(result, ctx);
	}

private:
	void formatNode(const T* node, std::string& result, const int depth) const {
		for (int i = 0; i < depth; ++i) {
			result += "  ";
		}
		result += std::format("{}\n", node->Id());

		for (const auto& child : node->Children()) {
			formatNode(child, result, depth + 1);
		}
	}
};
