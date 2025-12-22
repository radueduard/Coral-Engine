//
// Created by radue on 2/10/2025.
//

#pragma once

#include "ImGuizmo.h"
#include "core/input.h"
#include "ecs/scene.h"
#include "ecs/sceneManager.h"
#include "element.h"
#include "imgui.h"

#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include "ecs/entity.h"

namespace ImGui {
    static void RoundedImage(const ImTextureID user_texture_id,
        const ImVec2& size, const float diameter,
        const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1),
        const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
    {
        const ImVec2 p_min = GetCursorScreenPos();
        const ImVec2 p_max = {p_min.x + size.x, p_min.y + size.y };
        GetWindowDrawList()->AddImageRounded(
            user_texture_id,
            p_min, p_max,
            uv0, uv1,
            GetColorU32(tint_col), diameter * 0.5f
        );
        Dummy(ImVec2(diameter, diameter));
    }
}

namespace Coral::Reef {
    class Image final : public Element {
    public:
        explicit Image(const ImTextureID id, const Style& style = Style())
            : Element(style), m_texture(id) {}
        ~Image() override = default;

		void Subrender() override {
            const Math::Vector2f uv1 = { 0.f, 1.f };
            const Math::Vector2f uv2 = { 1.f, 0.f };

            ImGui::RoundedImage(
                m_texture,
                { m_currentSize.width - m_style.padding.left - m_style.padding.right, m_currentSize.height - m_style.padding.top - m_style.padding.bottom },
                m_style.cornerRadius,
                ImVec2(uv1), ImVec2(uv2));
        }

        void SetTexture(const ImTextureID id) {
            m_texture = id;
        }

    private:
        ImTextureID m_texture;
    };

	class MultiImage final : public Element {
	public:
		explicit MultiImage(const std::vector<ImTextureID>& ids, const Style& style = Style())
			: Element(style), m_textures(ids) {}
		~MultiImage() override = default;

		void Update() override {
			Element::Update();
			m_currentTextureIndex = ++m_currentTextureIndex % m_textures.size();
		}

		void Subrender() override {
			const Math::Vector2f uv1 = { 0.f, 1.f };
			const Math::Vector2f uv2 = { 1.f, 0.f };

			const auto& camera = ECS::SceneManager::Get().GetLoadedScene().MainCamera();

			Math::Matrix4f viewMatrix = camera.View();
			Math::Matrix4f projectionMatrix = camera.Projection();
			Math::Matrix4f modelMatrix = Math::Matrix4f::Identity();

			// grid size should increase with distance to camera
			constexpr float gridSize = 100.f;

			ImGuizmo::SetDrawlist();
			// set grid color to white with alpha 0.5
			ImGuizmo::GetStyle().Colors[ImGuizmo::COLOR::HATCHED_AXIS_LINES] = ImVec4(1.f, 1.f, 1.f, 0.5f);

			ImGuizmo::SetRect(
				ImGui::GetCursorScreenPos().x,
				ImGui::GetCursorScreenPos().y,
				m_currentSize.width - m_style.padding.left - m_style.padding.right,
				m_currentSize.height - m_style.padding.top - m_style.padding.bottom);

			ImGuizmo::DrawGrid(
				&viewMatrix[0][0],
				&projectionMatrix[0][0],
				&modelMatrix[0][0],
				gridSize);

			ImGui::RoundedImage(
				m_textures[m_currentTextureIndex],
				{ m_currentSize.width - m_style.padding.left - m_style.padding.right, m_currentSize.height - m_style.padding.top - m_style.padding.bottom },
				m_style.cornerRadius,
				ImVec2(uv1), ImVec2(uv2));

			const auto selectedEntity = ECS::SceneManager::Get().GetLoadedScene().SelectedEntity();
			if (!selectedEntity || selectedEntity->Id() == camera.Entity())
				return;

			static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			if (Input::IsKeyPressed(Key::B))
				currentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			if (Input::IsKeyPressed(Key::N))
				currentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
			if (Input::IsKeyPressed(Key::M))
				currentGizmoOperation = ImGuizmo::OPERATION::SCALE;

			static ImGuizmo::MODE currentGizmoMode = ImGuizmo::MODE::WORLD;
			if (Input::IsKeyPressed(Key::K))
				currentGizmoMode = ImGuizmo::MODE::LOCAL;
			if (Input::IsKeyPressed(Key::L))
				currentGizmoMode = ImGuizmo::MODE::WORLD;

			auto& transform = selectedEntity->Get<ECS::Transform>();
			const Math::Vector3f snapValue =
				currentGizmoOperation == ImGuizmo::OPERATION::ROTATE ?
					Math::Vector3f(15.f, 15.f, 15.f) :
					Math::Vector3f(0.5f, 0.5f, 0.5f);

			Math::Matrix4f entityModelMatrix = transform.Matrix();
			ImGuizmo::Manipulate(
				&viewMatrix[0][0],
				&projectionMatrix[0][0],
				currentGizmoOperation,
				currentGizmoMode,
				&entityModelMatrix[0][0],
				nullptr,
				Input::IsKeyHeld(Key::LeftControl) ? &snapValue[0] : nullptr,
				nullptr,
				nullptr
			);

			if (ImGuizmo::IsUsing()) {
				transform.SetTransform(entityModelMatrix);
			}
		}

		void SetTextures(const std::vector<ImTextureID>& ids, u32 startIndex = 0) {
			m_currentTextureIndex = startIndex;
			m_textures = ids;
		}

	private:
		u32 m_currentTextureIndex = 0;
		std::vector<ImTextureID> m_textures;
	};
}