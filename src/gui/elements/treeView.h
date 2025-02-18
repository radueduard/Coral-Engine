//
// Created by radue on 2/14/2025.
//

#pragma once

#include <imgui_internal.h>

#include "element.h"

namespace GUI {
    template<class T, class = std::enable_if_t<std::is_base_of_v<NarryTree<T>, T>>>
    class TreeView : public Element {
    public:
        explicit TreeView(T* tree) : m_tree(tree) {}
        ~TreeView() override = default;

        void Render() override {
            m_startPoint = m_parent->StartPoint(this);
            m_availableArea = m_parent->AllocatedArea(this);

            ImGui::SetCursorPos({ m_startPoint.x, m_startPoint.y });
            ImGui::BeginChild(typeid(T).name(), { m_availableArea.x, m_availableArea.y },
                ImGuiChildFlags_None,
                ImGuiWindowFlags_NoDecoration);

            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 34.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 11.0f, 5.0f });
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
            ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(32, 32, 32, 255));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(64, 64, 64, 255));
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(127, 127, 127, 255));

            RenderTree(m_tree);

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(8);
            ImGui::EndChild();
        }

    private:
        ImRect RenderTree(T* object)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
            if (object->Children().empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            std::string name = to_string(*object);

            const bool recurse = ImGui::TreeNodeEx((name + "##treenode").c_str(), flags, "%s  %s", ICON_FA_CUBE, name.c_str());


            ImGui::PushStyleColor(ImGuiCol_DragDropTarget, IM_COL32(255, 0, 0, 255));

            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("DND_OBJECT", &object, sizeof(T*), ImGuiCond_Once);

                if (ImGui::TreeNodeEx((name + "##treenode_dnd").c_str(), flags, "%s  %s", ICON_FA_CUBE, name.c_str()))
                    ImGui::TreePop();

                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const auto payload = ImGui::AcceptDragDropPayload("DND_OBJECT")) {
                    auto* recvObject = *static_cast<T**>(payload->Data);
                    if (recvObject != object) {
                        auto child = recvObject->Detach();
                        object->AddChild(std::move(child));
                    }
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::PopStyleColor();

            static std::pair<std::string, std::array<char, 256>> popupName;

            bool openPopup = false;
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Add Child")) {
                    openPopup = true;
                    popupName.first = name;
                    popupName.second.fill('\0');
                    memcpy(popupName.second.data(), "name", 4);
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

                if (ImGui::InputText("##name", popupName.second.data(), popupName.second.size())) {
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
                        for (const auto& child : object->Children()) {
                            if (to_string(*child) == _n) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            object->AddChild(std::make_unique<T>(popupName.second.data()));
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

                for (T* child : object->Children()) {
                    const ImRect childRect = RenderTree(child);
                    const auto midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;

                    if (child != object->Children().back()) {
                        drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + lineLengthX, midpoint), TreeLineColor, 2.0f);
                    } else {
                        drawList->AddLine(ImVec2(verticalLineStart.x + lineLengthX / 3.f, midpoint), ImVec2(verticalLineStart.x + lineLengthX, midpoint), TreeLineColor, 2.0f);
                    }
                    verticalLineEnd.y = midpoint;
                }
                if (!object->Children().empty()) {
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

        T* m_tree;
    };
}