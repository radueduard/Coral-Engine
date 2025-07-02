//
// Created by radue on 10/22/2024.
//

#include "input.h"

#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>


// MonoObject* GetMousePositionScripting() {
// 	auto mousePosition = Coral::Input::GetMousePosition();
// 	std::cout << "MousePosition: " << Coral::Input::GetMousePosition() << std::endl;
// }

namespace Coral {
    void Input::Setup() {
        for (const auto key : magic_enum::enum_values<Key>()) {
            m_keyboardKeyStates[key] = KeyState::NotPressed;
        }

        for (const auto button : magic_enum::enum_values<MouseButton>()) {
            m_mouseButtonStates[button] = KeyState::NotPressed;
        }
    }

    void Input::Update() {
        for (const auto key : magic_enum::enum_values<Key>()) {
            if (m_keyboardKeyStates[key] == KeyState::Pressed) {
                m_keyboardKeyStates[key] = KeyState::Held;
            } else if (m_keyboardKeyStates[key] == KeyState::Released) {
                m_keyboardKeyStates[key] = KeyState::NotPressed;
            }
        }

        for (const auto button : magic_enum::enum_values<MouseButton>()) {
            if (m_mouseButtonStates[button] == KeyState::Pressed) {
                m_mouseButtonStates[button] = KeyState::Held;
            } else if (m_mouseButtonStates[button] == KeyState::Released) {
                m_mouseButtonStates[button] = KeyState::NotPressed;
            }
        }

        m_mouseDelta = { 0, 0 };
        m_scrollDelta = { 0.0f, 0.0f };
    }

    KeyState Input::GetKeyState(const Key key) {
        return m_keyboardKeyStates[key];
    }

    KeyState Input::GetMouseButtonState(const MouseButton button) {
        return m_mouseButtonStates[button];
    }

    bool Input::IsKeyPressed(const Key key) {
        return m_keyboardKeyStates[key] == KeyState::Pressed;
    }

    bool Input::IsKeyHeld(const Key key) {
        return m_keyboardKeyStates[key] == KeyState::Held;
    }

    bool Input::IsKeyReleased(const Key key) {
        return m_keyboardKeyStates[key] == KeyState::Released;
    }

    bool Input::IsMouseButtonPressed(const MouseButton button) {
        return m_mouseButtonStates[button] == KeyState::Pressed;
    }

    bool Input::IsMouseButtonHeld(const MouseButton button) {
        return m_mouseButtonStates[button] == KeyState::Held;
    }

    bool Input::IsMouseButtonReleased(const MouseButton button) {
        return m_mouseButtonStates[button] == KeyState::Released;
    }

    Math::Vector2<i32>& Input::GetMousePosition() {
        return m_mousePosition;
    }

    Math::Vector2<i32>& Input::GetMousePositionDelta() {
        return m_mouseDelta;
    }

    Math::Vector2<f32>& Input::GetMouseScrollDelta() {
        return m_scrollDelta;
    }

	void Input::Callbacks::keyCallback(GLFWwindow *, int key, int, const int action, int) {
    	const auto k = static_cast<Key>(key);

    	switch (action) {
    	case GLFW_PRESS:
    		m_keyboardKeyStates[k] = KeyState::Pressed;
    		break;
    	case GLFW_RELEASE:
    		m_keyboardKeyStates[k] = KeyState::Released;
    		break;
    	default:
    		break;
    	}
    }

	void Input::Callbacks::mouseMoveCallback(GLFWwindow *, const double x, const double y) {
    	const Math::Vector2 pos { static_cast<i32>(x), static_cast<i32>(y) };
    	m_mouseDelta = pos - m_mousePosition;
    	m_mousePosition = pos;

    }

	void Input::Callbacks::mouseButtonCallback(GLFWwindow *, int button, const int action, int) {
    	const auto b = static_cast<MouseButton>(button);
    	switch (action) {
    	case GLFW_PRESS:
    		m_mouseButtonStates[b] = KeyState::Pressed;
    		break;
    	case GLFW_RELEASE:
    		m_mouseButtonStates[b] = KeyState::Released;
    		break;
    	default:
    		break;
    	}
    }

	void Input::Callbacks::scrollCallback(GLFWwindow *, const double x, const double y) {
    	m_scrollDelta = { static_cast<f32>(x), static_cast<f32>(y) };
    }
}
