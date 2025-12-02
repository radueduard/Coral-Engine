//
// Created by radue on 2/10/2025.
//

#pragma once

#include "math/vector.h"
#include <imgui.h>

#include <functional>

#include "color/color.h"
#include "math/constants.h"
#include "gui/padding.h"

#include "context.h"

namespace Coral::Reef {

    class ElementList;

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
        f32 cornerRadius = padding.right;
        Color backgroundColor = Colors::transparent;
        Axis direction = Axis::Horizontal;
    	bool allowInteraction = true;
        bool debug = false;
    };

    class Element {
    public:
        explicit Element(const Style& style = {}, const std::vector<Element*>& children = {});
        virtual ~Element() = default;

        Element(const Element&) = default;
        Element& operator=(const Element&) = default;

    	void AddChild(Element* child);

        [[nodiscard]] const Math::Vector2<f32>& RelativePosition() const { return m_relativePosition; }
        [[nodiscard]] const Math::Vector2<f32>& AbsolutePosition() const { return m_absolutePosition; }

        f32 ComputeFitSizeOnAxis(Axis axis);
        void ComputeGrowSizeOnAxis(Axis axis);
        void SetPosition(Math::Vector2<f32> position);

        [[nodiscard]] bool RecreateRequired() const;
        virtual void Render();

        virtual void ComputeLayout();
        virtual void Update();
    	virtual void Subrender() {}

    	void DisableInteraction() {
    		m_style.allowInteraction = false;
    		for (const auto& child : m_children) {
				child->DisableInteraction();
			}
    	}

        void Debug() const;

        [[nodiscard]]
    	const Math::Vector2<f32>& CurrentSize() const;

    protected:
        f32& CurrentSize(Axis axis);

        f32& BaseSize(Axis axis);

        f32& ChildrenSize(Axis axis);

        [[nodiscard]]
        f32 Padding(Axis axis) const;

        [[nodiscard]]
        bool IsGrowOnAxis(Axis axis) const;

        [[nodiscard]]
        bool IsShrinkOnAxis(Axis axis) const;
        void ResetState(const std::function<void()>& resetFunc = [] {});

        UUID m_uuid;
        Style m_style;

        Math::Vector2<f32> m_baseSize;
        Math::Vector2<f32> m_minSize;

        Math::Vector2<f32> m_childrenSize;
        Math::Vector2<f32> m_currentSize;
        Math::Vector2<f32> m_relativePosition = { 0.f, 0.f };
        Math::Vector2<f32> m_absolutePosition = { 0.f, 0.f };
        Math::Vector2<f32> m_actualRenderedPosition = { 0.f, 0.f };

        bool m_shouldResize = true;

        Element* m_parent = nullptr;
        std::vector<std::unique_ptr<Element>> m_children;

    };
}
