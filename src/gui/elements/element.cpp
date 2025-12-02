//
// Created by radueduard on 11/2/25.
//

#include "element.h"

namespace Coral::Reef {
    Element::Element(const Style& style, const std::vector<Element*>& children)
        : m_style(style){
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

    void Element::AddChild(Element *child) {
        child->m_parent = this;
        m_children.emplace_back(child);
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

            // for (const auto& child : m_children) {
            //     if (child->IsShrinkOnAxis(axis)) {
            //         remainingSize += child->CurrentSize(axis) - child->BaseSize(axis);
            //         child->CurrentSize(axis) = child->BaseSize(axis);
            //     }
            // }

            if (remainingSize <= -Math::Epsilon<f32>()) {
                for (const auto& child : m_children) {
                    if (child->IsShrinkOnAxis(axis)) {
                        child->CurrentSize(axis) = std::min(CurrentSize(axis) - Padding(axis), child->CurrentSize(axis));
                    }
                }

                for (const auto& child : m_children) {
                    child->ComputeGrowSizeOnAxis(axis);
                }
                return;
            }

            const size_t growableChildren = std::ranges::count_if(
                m_children,
                [axis](const auto& child) {
                    return child->IsGrowOnAxis(axis);
                }
            );

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
                if (child->IsShrinkOnAxis(axis)) {
                    child->CurrentSize(axis) = std::min(CurrentSize(axis) - Padding(axis), child->CurrentSize(axis));
                }
            }
        }

        for (const auto& child : m_children) {
            child->ComputeGrowSizeOnAxis(axis);
        }
    }

    void Element::SetPosition(Math::Vector2<f32> position)
    {
        m_relativePosition = position;
        m_absolutePosition = (m_parent ? m_parent->m_absolutePosition : m_absolutePosition) + position;

        position.x = m_style.padding.left;
        position.y = m_style.padding.top;

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

    bool Element::RecreateRequired() const { return m_shouldResize; }

    void Element::ComputeLayout()
    {
        ComputeFitSizeOnAxis(Axis::Horizontal);
        ComputeFitSizeOnAxis(Axis::Vertical);
        ComputeGrowSizeOnAxis(Axis::Horizontal);
        ComputeGrowSizeOnAxis(Axis::Vertical);
        SetPosition(m_relativePosition);
    }

    void Element::Update()
    {
        for (const auto& child : m_children) {
            child->Update();
        }

        m_shouldResize = std::ranges::any_of(m_children, [](const auto& child) {
            return child->RecreateRequired();
        });
    }

    void Element::Render()
    {
        if (m_style.debug) {
            Debug();
        }

        auto cornerRadius = m_style.cornerRadius;
        if (const auto smallerDimension = std::min(m_currentSize.width, m_currentSize.height);
            cornerRadius * 2 > smallerDimension)
        {
            cornerRadius = smallerDimension / 2.f;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_style.padding.left, m_style.padding.top));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, cornerRadius);
        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            ImGui::ColorConvertFloat4ToU32(ImVec4(m_style.backgroundColor))
        );

        int windowFlags = ImGuiWindowFlags_NoDecoration;
        if (!m_style.allowInteraction) {
            windowFlags |= ImGuiWindowFlags_NoMouseInputs;
        }

        ImGui::SetCursorPos({ m_relativePosition.x, m_relativePosition.y });
        ImGui::BeginChild(
            boost::uuids::to_string(m_uuid).c_str(),
            ImVec2(m_currentSize.width, m_currentSize.height),
            ImGuiChildFlags_AlwaysUseWindowPadding,
            windowFlags
        );

        m_actualRenderedPosition = Math::Vector2<f32>(ImGui::GetCursorScreenPos());

        Subrender();

        for (const auto& child : m_children) {
            child->Render();
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    void Element::Debug() const
    {
        const auto drawList = ImGui::GetWindowDrawList();

        drawList->AddRect(
            ImVec2(m_absolutePosition),
            ImVec2(m_absolutePosition + m_currentSize),
            IM_COL32(0, 0, 255, 255),
            m_style.cornerRadius
        );

        ImGui::Begin(boost::uuids::to_string(m_uuid).c_str());
        ImGui::Text("Window absolute position: %f, %f", m_absolutePosition.x, m_absolutePosition.y);
        ImGui::Text("Window relative position: %f, %f", m_relativePosition.x, m_relativePosition.y);
        ImGui::Text("Window size: %f, %f", m_currentSize.x, m_currentSize.y);
        ImGui::Text("Window base size: %f, %f", m_baseSize.x, m_baseSize.y);
        ImGui::Text("Window min size: %f, %f", m_minSize.x, m_minSize.y);
        ImGui::NewLine();
        ImGui::End();
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
