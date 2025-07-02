//
// Created by radue on 2/10/2025.
//

#pragma once

#include "math/vector.h"
#include <imgui.h>

#include <functional>

#include "math/constants.h"

namespace Coral::Reef {
    struct Padding {
        f32 left = 0.f;
        f32 right = 0.f;
        f32 top = 0.f;
        f32 bottom = 0.f;

    	Padding() = default;
    	explicit Padding(const f32 value)
			: left(value), right(value), top(value), bottom(value) {}
    	Padding(const f32 vertical, const f32 horizontal)
    		: left(horizontal), right(horizontal), top(vertical), bottom(vertical) {}
    	Padding(const f32 left, const f32 right, const f32 top, const f32 bottom)
    		: left(left), right(right), top(top), bottom(bottom) {}
    };


	enum class Axis : bool {
		Horizontal,
		Vertical,
	};

    inline static constexpr f32 Grow = 0.f;
    inline static constexpr f32 Shrink = -1.f;

    struct Style {
        Math::Vector2<f32> size = { Grow, Grow };
        Padding padding = { 0.f, 0.f, 0.f, 0.f };
        f32 spacing = ImGui::GetStyle().ItemSpacing.x;
        f32 cornerRadius = ImGui::GetStyle().FrameRounding;
        Color backgroundColor = { 0.f, 0.f, 0.f, 0.f };
        Axis direction = Axis::Horizontal;
    	bool scrollable = false;
    };

    class Element {
        friend class LabeledRow;
        friend class Conditional;
    public:
        explicit Element(const Style& style = {}, const std::vector<Element*>& children = {}) {
            m_baseSize = style.size;
            m_axis = style.direction;
        	m_scrollable = style.scrollable;
            m_padding = style.padding;
            m_spacing = style.spacing;
            m_cornerRadius = style.cornerRadius;
            m_backgroundColor = style.backgroundColor;
            m_children.reserve(children.size());
            for (const auto child : children) {
                child->m_parent = this;
                m_children.emplace_back(child);
            }
        }
        virtual ~Element() = default;

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;

        [[nodiscard]] const Math::Vector2<f32>& Position() const { return m_position; }

        f32 ComputeFitSizeOnAxis(const Axis axis) {
            if (axis == m_axis) {
                CurrentSize(axis) = BaseSize(axis);
                ChildrenSize(axis) = 0.f;

                for (const auto& child : m_children) {
                    ChildrenSize(axis) += child->ComputeFitSizeOnAxis(axis);
                }
                if (!m_children.empty()) {
                    ChildrenSize(axis) += m_spacing * static_cast<f32>(m_children.size() - 1);
                }
            } else {
                CurrentSize(axis) = BaseSize(axis);
                ChildrenSize(axis) = 0.f;

                for (const auto& child : m_children) {
                    ChildrenSize(axis) = std::max(ChildrenSize(axis), child->ComputeFitSizeOnAxis(axis));
                }
            }
            if (IsGrowOnAxis(axis) || IsShrinkOnAxis(axis)) {
                CurrentSize(axis) = ChildrenSize(axis) + Padding(axis);
            } else {
                CurrentSize(axis) = BaseSize(axis);
            }

            return CurrentSize(axis);
        }

        void ComputeGrowSizeOnAxis(const Axis axis) {
            if (axis == m_axis) {
                f32 remainingSize = CurrentSize(axis) - ChildrenSize(axis) - Padding(axis);

                i32 growableChildren = 0;
                for (const auto& child : m_children) {
                    if (child->IsGrowOnAxis(axis)) {
                        ++growableChildren;
                    }
                }

                while (remainingSize > Math::Epsilon<f32>() && growableChildren > 0) {
                    f32 smallestSize = std::numeric_limits<f32>::max();
                    f32 secondSmallestSize = std::numeric_limits<f32>::max();
                    f32 sizeToAdd = remainingSize;
                    for (const auto& child : m_children) {
                        if (child->IsGrowOnAxis(axis)) {
                            f32 size = child->CurrentSize(axis);
                            if (size < smallestSize) {
                                secondSmallestSize = smallestSize;
                                smallestSize = size;
                            } else if (size > smallestSize) {
                                secondSmallestSize = std::min(secondSmallestSize, size);
                                sizeToAdd = secondSmallestSize - smallestSize;
                            }
                        }
                    }

                    sizeToAdd = std::min(sizeToAdd, remainingSize / static_cast<f32>(growableChildren));

                    for (const auto& child : m_children) {
                        if (child->IsGrowOnAxis(axis)) {
                            f32 size = child->CurrentSize(axis);
                            if (size == smallestSize) {
                                child->CurrentSize(axis) += sizeToAdd;
                                remainingSize -= sizeToAdd;
                            }
                        }
                    }
                }
            } else {
                for (const auto& child : m_children) {
                    if (child->IsGrowOnAxis(axis)) {
                        child->CurrentSize(axis) = CurrentSize(axis) - Padding(axis);
                    }
                }
            }

            for (const auto& child : m_children) {
                child->ComputeGrowSizeOnAxis(axis);
            }
        }

        void SetPosition(Math::Vector2<f32> position) {
			m_position = position;
			position.x += m_padding.left;
			position.y += m_padding.top;

			for (const auto& child : m_children) {
				child->SetPosition(position);
				if (m_axis == Axis::Horizontal) {
					position.x += child->CurrentSize(m_axis) + m_spacing;
				}
				else {
					position.y += child->CurrentSize(m_axis) + m_spacing;
				}
			}
		}

    	virtual void RecreateRequired() {
			for (const auto& child : m_children) {
				child->RecreateRequired();
			}
		}

		virtual void ComputeLayout() {
        	RecreateRequired();
            ComputeFitSizeOnAxis(Axis::Horizontal);
            ComputeFitSizeOnAxis(Axis::Vertical);
            ComputeGrowSizeOnAxis(Axis::Horizontal);
            ComputeGrowSizeOnAxis(Axis::Vertical);
            SetPosition(m_position);
        }

        virtual bool Render() {
            ImGui::SetCursorScreenPos({ m_position.x, m_position.y });
            const auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(
                { m_position.x, m_position.y },
                { m_position.x + m_currentSize.width, m_position.y + m_currentSize.height },
                ImGui::ColorConvertFloat4ToU32({ m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a }),
                m_cornerRadius
            );

        	if (m_scrollable) {
        		ImGui::BeginChild(
					("##" + std::to_string(reinterpret_cast<std::uintptr_t>(this))).c_str(),
					ImVec2(m_currentSize.width - m_padding.left - m_padding.right, m_currentSize.height - m_padding.top - m_padding.bottom),
					false,
					ImGuiWindowFlags_NoScrollbar
				);
        	}
        	bool shouldReset = false;
            for (const auto& child : m_children) {
				if (child == nullptr)
					continue;
            	shouldReset |= child->Render();
            }
        	if (m_scrollable) {
				ImGui::EndChild();
			}
        	return shouldReset;
        }

        void Outline() const {
            const auto drawList = ImGui::GetWindowDrawList();

            drawList->AddRect(
                ImVec2(m_position),
                ImVec2(m_position + m_currentSize),
                IM_COL32(0, 0, 255, 255),
                m_cornerRadius
            );
        }

    	[[nodiscard]]
    	const Math::Vector2<f32>& CurrentSize() const {
	        return m_currentSize;
        }

    protected:
        f32& CurrentSize(const Axis axis) {
            if (axis == Axis::Horizontal) {
                return m_currentSize.width;
            } else {
                return m_currentSize.height;
            }
        }

        f32& BaseSize(const Axis axis) {
            if (axis == Axis::Horizontal) {
                return m_baseSize.width;
            } else {
                return m_baseSize.height;
            }
        }

        f32& ChildrenSize(const Axis axis) {
            if (axis == Axis::Horizontal) {
                return m_childrenSize.height;
            } else {
                return m_childrenSize.width;
            }
        }

        [[nodiscard]]
        f32 Padding(const Axis axis) const {
            if (axis == Axis::Horizontal) {
                return m_padding.left + m_padding.right;
            } else {
                return m_padding.top + m_padding.bottom;
            }
        }

        [[nodiscard]]
        bool IsGrowOnAxis(const Axis axis) const {
            if (axis == Axis::Horizontal) {
                return m_baseSize.width == Grow;
            } else {
                return m_baseSize.height == Grow;
            }
        }

        [[nodiscard]]
        bool IsShrinkOnAxis(const Axis axis) const {
            if (axis == Axis::Horizontal) {
                return m_baseSize.width <= Shrink;
            } else {
                return m_baseSize.height <= Shrink;
            }
        }

        void ResetState(const std::function<void()>& resetFunc = [] {}) {
            resetFunc();
            Element* root = this;
            while (root->m_parent != nullptr) {
                root = root->m_parent;
            }
            root->ComputeLayout();
        }

        Reef::Padding m_padding;
        f32 m_spacing = 0.f;
        f32 m_cornerRadius = 0.f;
        Axis m_axis = Axis::Horizontal;
    	bool m_scrollable = false;
        Color m_backgroundColor;

        Math::Vector2<f32> m_baseSize;
        Math::Vector2<f32> m_childrenSize;
        Math::Vector2<f32> m_currentSize;
        Math::Vector2<f32> m_position = { 0.f, 0.f };

        Element* m_parent = nullptr;
        std::vector<std::unique_ptr<Element>> m_children;

    };
}
