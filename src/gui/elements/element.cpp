//
// Created by radueduard on 11/2/25.
//

#include "element.h"

namespace Coral::Reef {
    Element::Element(const Style& style, const std::vector<Element*> children, const bool debug)
        : m_style(style), m_debug(debug) {
        m_children.reserve(children.size());
        for (auto& child : children) {
            m_children.emplace_back(child);
        }

        m_baseSize = style.size;
        m_minSize = { 0.f, 0.f };
        m_uuid = Context::GenerateUUID();

        for (auto& child : children) {
            child->m_parent = this;
            if (m_style.direction == Axis::Horizontal) {
                m_minSize.width += child->m_minSize.width;
                m_minSize.height = std::max(m_minSize.height, child->m_minSize.height);
            } else {
                m_minSize.width = std::max(m_minSize.width, child->m_minSize.width);
                m_minSize.height += child->m_minSize.height;
            }
        }

        const auto childCount = static_cast<int>(m_children.size());
        if (childCount == 0) return;

        m_minSize += { m_style.padding.Horizontal(), m_style.padding.Vertical() };

        if (m_style.direction == Axis::Horizontal) {
            m_minSize.width += m_style.spacing * static_cast<f32>(childCount - 1);
        } else {
            m_minSize.height += m_style.spacing * static_cast<f32>(childCount - 1);
        }
    }

    f32 Element::ComputeFitSizeOnAxis(const Axis axis)
    {
        if (axis == m_style.direction) {
            CurrentSize(axis) = BaseSize(axis);
            ChildrenSize(axis) = 0.f;

            for (const auto& child : m_children) {
                ChildrenSize(axis) += child->ComputeFitSizeOnAxis(axis);
            }
            if (!m_children.empty()) {
                ChildrenSize(axis) += m_style.spacing * static_cast<f32>(m_children.size() - 1);
            }
        } else {
            CurrentSize(axis) = BaseSize(axis);
            ChildrenSize(axis) = 0.f;

            for (auto& child : m_children) {
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

    void Element::ComputeGrowSizeOnAxis(const Axis axis)
    {
        if (axis == m_style.direction) {
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
                        if (f32 size = child->CurrentSize(axis); size < smallestSize) {
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
                        if (const f32 size = child->CurrentSize(axis); size == smallestSize) {
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

        for (auto& child : m_children) {
            child->ComputeGrowSizeOnAxis(axis);
        }
    }

    void Element::SetPosition(Math::Vector2<f32> position)
    {
        m_position = position;
        position.x += m_style.padding.left;
        position.y += m_style.padding.top;

        for (const auto& child : m_children) {
            child->SetPosition(position);
            if (m_style.direction == Axis::Horizontal) {
                position.x += child->CurrentSize(m_style.direction) + m_style.spacing;
            }
            else {
                position.y += child->CurrentSize(m_style.direction) + m_style.spacing;
            }
        }
    }

    bool Element::RecreateRequired()
    {
        bool recreateRequired = false;
        for (const auto& child : m_children) {
            recreateRequired |= child->RecreateRequired();
        }
        if (recreateRequired) {
            m_minSize = { 0.f, 0.f };
            for (const auto& child : m_children) {
                if (m_style.direction == Axis::Horizontal) {
                    m_minSize.width += child->m_minSize.width;
                    m_minSize.height = std::max(m_minSize.height, child->m_minSize.height);
                } else {
                    m_minSize.width = std::max(m_minSize.width, child->m_minSize.width);
                    m_minSize.height += child->m_minSize.height;
                }
            }
            const auto childCount = static_cast<int>(m_children.size());
            if (childCount == 0) return recreateRequired;

            m_minSize += { m_style.padding.Horizontal(), m_style.padding.Vertical() };

            if (m_style.direction == Axis::Horizontal) {
                m_minSize.width += m_style.spacing * static_cast<f32>(childCount - 1);
            } else {
                m_minSize.height += m_style.spacing * static_cast<f32>(childCount - 1);
            }
        }
        return recreateRequired;
    }

    void Element::ComputeLayout()
    {
        ComputeFitSizeOnAxis(Axis::Horizontal);
        ComputeFitSizeOnAxis(Axis::Vertical);
        ComputeGrowSizeOnAxis(Axis::Horizontal);
        ComputeGrowSizeOnAxis(Axis::Vertical);
        SetPosition(m_position);
    }

    bool Element::Render()
    {
        ImGui::SetCursorScreenPos({ m_position.x + m_style.padding.left, m_position.y + m_style.padding.top });
        const auto drawList = ImGui::GetWindowDrawList();

        auto cornerRadius = m_style.cornerRadius;
        if (const auto smallerDimension = std::min(m_currentSize.width, m_currentSize.height);
            cornerRadius * 2 > smallerDimension)
        {
            cornerRadius = smallerDimension / 2.f;
        }

        drawList->AddRectFilled(
            { m_position.x, m_position.y },
            { m_position.x + m_currentSize.width, m_position.y + m_currentSize.height },
            ImGui::ColorConvertFloat4ToU32(ImVec4(m_style.backgroundColor)),
            cornerRadius
        );

        if (m_debug) {
            ImGui::Begin("Debug");
            ImGui::Text("Window position: %f, %f", m_position.x, m_position.y);
            ImGui::Text("Window size: %f, %f", m_currentSize.x, m_currentSize.y);
            ImGui::Text("Window base size: %f, %f", m_baseSize.x, m_baseSize.y);
            ImGui::Text("Window min size: %f, %f", m_minSize.x, m_minSize.y);
            ImGui::NewLine();
            ImGui::End();
        }

        // ImGui::PushStyleColor(ImGuiCol_ChildBg, static_cast<ImVec4>(m_style.backgroundColor));
        // ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, m_style.cornerRadius);
        // // ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(m_style.padding.left, m_style.padding.top));
        // // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_style.padding.left, m_style.padding.top));
        //
        // ImGui::BeginChild(
        //     ("##" + boost::uuids::to_string(m_uuid)).c_str(),
        //     ImVec2(m_currentSize.width, m_currentSize.height),
        //     ImGuiChildFlags_AlwaysUseWindowPadding,
        //     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        // );


        bool shouldReset = false;
        for (const auto& child : m_children) {
            shouldReset |= child->Render();
        }

    	if (m_style.debug) {
    		Outline();
    	}

        // ImGui::EndChild();
        //
        // ImGui::PopStyleVar();
        // ImGui::PopStyleColor();

        return shouldReset;
    }

    void Element::Outline() const
    {
        const auto drawList = ImGui::GetWindowDrawList();

        drawList->AddRect(
            ImVec2(m_position),
            ImVec2(m_position + m_currentSize),
            IM_COL32(0, 0, 255, 255),
            m_style.cornerRadius
        );
    }

    const Math::Vector2<f32>& Element::CurrentSize() const
    {
        return m_currentSize;
    }

    f32& Element::CurrentSize(const Axis axis)
    {
        if (axis == Axis::Horizontal) {
            return m_currentSize.width;
        } else {
            return m_currentSize.height;
        }
    }

    f32& Element::BaseSize(const Axis axis)
    {
        if (axis == Axis::Horizontal) {
            return m_baseSize.width;
        } else {
            return m_baseSize.height;
        }
    }

    f32& Element::ChildrenSize(const Axis axis)
    {
        if (axis == Axis::Horizontal) {
            return m_childrenSize.height;
        } else {
            return m_childrenSize.width;
        }
    }

    f32 Element::Padding(const Axis axis) const
    {
        if (axis == Axis::Horizontal) {
            return m_style.padding.left + m_style.padding.right;
        } else {
            return m_style.padding.top + m_style.padding.bottom;
        }
    }

    bool Element::IsGrowOnAxis(const Axis axis) const
    {
        if (axis == Axis::Horizontal) {
            return m_baseSize.width == Grow;
        } else {
            return m_baseSize.height == Grow;
        }
    }

    bool Element::IsShrinkOnAxis(const Axis axis) const
    {
        if (axis == Axis::Horizontal) {
            return m_baseSize.width <= Shrink;
        } else {
            return m_baseSize.height <= Shrink;
        }
    }

    void Element::ResetState(const std::function<void()>& resetFunc)
    {
        resetFunc();
        auto root = this;
        while (root->m_parent != nullptr) {
            root = root->m_parent;
        }
        root->ComputeLayout();
    }
}
