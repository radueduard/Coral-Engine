//
// Created by radue on 2/14/2025.
//

#pragma once

#include <imgui_internal.h>

#include "utils/narryTree.h"
#include "element.h"
#include "math/rect.h"

#include "contextMenu.h"
#include "IconsFontAwesome6.h"

#include "ecs/scene.h"

namespace Coral::Reef {
    template<typename T, typename IdType> requires std::is_base_of_v<NarryTree<T, IdType>, T>
    class TreeView final : public Element {
    public:
        TreeView(T& tree, std::function<void(T&)> onItemClick, const Style& style = Style())
            : Element(style), m_tree(tree), m_onItemClick(std::move(onItemClick)) {}
        ~TreeView() override = default;

		bool Render() override {
			const bool shouldReset = Element::Render();

            ImGui::BeginChildFrame(ImGui::GetID(this), ImVec2(m_currentSize), ImGuiWindowFlags_NoBackground);
            ImGui::EndChildFrame();
            if (ImGui::BeginDragDropTarget()) {
                if (const auto payload = ImGui::AcceptDragDropPayload("DND_OBJECT")) {
                    IdType recvObject = *static_cast<IdType*>(payload->Data);
                    if (m_tree.Id() != recvObject) {
                        T* recv = ECS::Scene::Get().Registry().get<T*>(recvObject);
                        if (recv != nullptr) {
                            auto detached = recv->Detach();
                            m_tree.AddChild(std::move(detached));
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SetCursorScreenPos(ImVec2(m_position + Math::Vector2 { m_padding.left, m_padding.top }));
        	const auto childSize = m_currentSize - Math::Vector2 { m_padding.left + m_padding.right, m_padding.top + m_padding.bottom };
            ImGui::BeginChild(
            	typeid(T).name(),
                ImVec2(childSize),
                ImGuiChildFlags_None,
                ImGuiWindowFlags_NoDecoration);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
            for (auto& object : m_tree.Children()) {
                const auto nodeRect = RenderTree(*object);
                if (nodeRect.Contains(ImGui::GetMousePos())) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
            }

            ImGui::PopStyleVar();
            ImGui::EndChild();

			return shouldReset;
        }

    private:
        ImRect RenderTree(T& object) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
            if (object.Children().empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            const std::string name = to_string(object);
            const bool recurse = ImGui::TreeNodeEx((name + "##treenode").c_str(), flags, "%s   %s", ICON_FA_CUBE, name.c_str());

            if (ImGui::IsItemClicked()) {
                m_onItemClick(object);
            }

            ImGui::PushStyleColor(ImGuiCol_DragDropTarget, IM_COL32(255, 0, 0, 255));

            if (ImGui::BeginDragDropSource()) {
                auto id = object.Id();
                ImGui::SetDragDropPayload("DND_OBJECT", &id, sizeof(IdType), ImGuiCond_Once);

                if (ImGui::TreeNodeEx((name + "##treenode_dnd").c_str(), flags, "%s   %s", ICON_FA_CUBE, name.c_str()))
                    ImGui::TreePop();

                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const auto payload = ImGui::AcceptDragDropPayload("DND_OBJECT")) {
                    IdType recvObject = *static_cast<IdType*>(payload->Data);
                    if (object.Id() != recvObject) {
                        T* recv = ECS::Scene::Get().Registry().get<T*>(recvObject);
                        if (recv != nullptr) {
                            try {
                                recv->FindChild(object.Id());
                            } catch (const std::exception& e) {
                                auto detached = recv->Detach();
                                object.AddChild(std::move(detached));
                            }
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PopStyleColor();

            static std::pair<std::string, std::array<char, 256>> popupName;

            bool openPopup = false;
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::Selectable(ICON_FA_PLUS "    Add Child", false)) {
                    openPopup = true;
                    popupName.first = name;
                    popupName.second.fill('\0');
                }
                if (ImGui::Selectable(ICON_FA_TRASH "    Delete", false)) {
                    object.Detach();
                }
                ImGui::EndPopup();
            }
            if (openPopup) {
                ImGui::OpenPopup((name + "##popup").c_str());
            }

            const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            if (ImGui::BeginPopupModal((name + "##popup").c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("New object").x) / 2.0f);
                ImGui::Text("New object");

                static bool displayErrorMessage = false;

                if (ImGui::IsWindowAppearing()) {
                    ImGui::SetKeyboardFocusHere();
                }

                if (ImGui::InputTextEx("##name", "name", popupName.second.data(), popupName.second.size(), ImVec2(0, 0), ImGuiInputTextFlags_None)) {
                    displayErrorMessage = false;
                }

                if (displayErrorMessage) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

                    ImGui::TextWrapped("Object with this name already exists under this parent");

                    ImGui::PopStyleColor();
                }

                const float buttonWidth = 2 * ImGui::GetStyle().FramePadding.x + ImGui::CalcTextSize("Cancel").x + 10.0f;
                const float buttonAreaWidth = buttonWidth * 2 + ImGui::GetStyle().ItemSpacing.x;

                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonAreaWidth) / 2.0f);

                if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                    ImGui::CloseCurrentPopup();
                    displayErrorMessage = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Add", ImVec2(buttonWidth, 0))) {
                    if (popupName.second[0] != '\0') {
                        const auto _n = std::string(popupName.second.data());
                        bool exists = false;
                        for (const auto& child : object.Children()) {
                            if (to_string(*child) == _n) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            object.AddChild(std::make_unique<T>(popupName.second.data()));
                            ImGui::CloseCurrentPopup();
                        } else {
                            displayErrorMessage = true;
                        }
                    }
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();

            const auto nodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

            if (recurse) {
                constexpr ImColor TreeLineColor = IM_COL32(127, 127, 127, 255);

                constexpr float leftToArrow = 17.f;
                const float lineLengthX = ImGui::GetStyle().IndentSpacing - leftToArrow;
                const float smallOffsetY = ImGui::GetStyle().ItemSpacing.y;
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
                verticalLineStart.x -= lineLengthX;
                verticalLineStart.y -= smallOffsetY;
                ImVec2 verticalLineEnd = verticalLineStart;

                auto children = object.Children();
                for (T* child : children) {
                    const ImRect childRect = RenderTree(*child);
                    const auto midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;

                    if (child != children.back()) {
                        drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + lineLengthX, midpoint), TreeLineColor, 2.0f);
                    } else {
                        drawList->AddLine(ImVec2(verticalLineStart.x + lineLengthX / 3.f, midpoint), ImVec2(verticalLineStart.x + lineLengthX, midpoint), TreeLineColor, 2.0f);
                    }
                    verticalLineEnd.y = midpoint;
                }
                if (!children.empty()) {
                    verticalLineEnd.y -= lineLengthX / 3.f;
                    drawList->PathLineTo(verticalLineStart);
                    drawList->PathLineTo(verticalLineEnd);
                    drawList->PathStroke(TreeLineColor, ImDrawFlags_None, 2.0f);
                    drawList->PathClear();
                    const auto arcCenter = ImVec2(verticalLineEnd.x + lineLengthX / 3.f, verticalLineEnd.y);
                    drawList->PathArcTo(arcCenter, lineLengthX / 3.f, IM_PI / 2.f, IM_PI + 0.1f);
                    drawList->PathStroke(TreeLineColor, ImDrawFlags_None, 2.0f);
                    drawList->PathClear();
                }

                ImGui::TreePop();
            }

            return nodeRect;
        }

        T& m_tree;
        std::function<void(T&)> m_onItemClick;
        std::unique_ptr<ContextMenu> m_contextMenu;
    };
}
