//
// Created by radue on 10/22/2024.
//

#include "input.h"

#include <unordered_set>

std::unordered_set allKeys = {
	Key::Space,
	Key::Apostrophe,
	Key::Comma,
	Key::Minus,
	Key::Period,
	Key::Slash,
	Key::Num0,
	Key::Num1,
	Key::Num2,
	Key::Num3,
	Key::Num4,
	Key::Num5,
	Key::Num6,
	Key::Num7,
	Key::Num8,
	Key::Num9,
	Key::Semicolon,
	Key::Equal,
	Key::A,
	Key::B,
	Key::C,
    Key::D,
    Key::E,
    Key::F,
    Key::G,
    Key::H,
    Key::I,
    Key::J,
    Key::K,
    Key::L,
    Key::M,
	Key::N,
    Key::O,
    Key::P,
    Key::Q,
    Key::R,
    Key::S,
    Key::T,
    Key::U,
    Key::V,
    Key::W,
    Key::X,
    Key::Y,
    Key::Z,
    Key::LeftBracket,
    Key::Backslash,
    Key::RightBracket,
    Key::GraveAccent,
    Key::World1,
    Key::World2,
    Key::Esc,
    Key::Enter,
    Key::Tab,
    Key::Backspace,
    Key::Insert,
    Key::Delete,
    Key::Right,
    Key::Left,
    Key::Down,
    Key::Up,
    Key::PageUp,
    Key::PageDown,
    Key::Home,
    Key::End,
    Key::CapsLock,
    Key::ScrollLock,
    Key::NumLock,
    Key::PrintScreen,
    Key::Pause,
    Key::F1,
    Key::F2,
    Key::F3,
    Key::F4,
    Key::F5,
    Key::F6,
    Key::F7,
    Key::F8,
    Key::F9,
    Key::F10,
    Key::F11,
    Key::F12,
    Key::F13,
    Key::F14,
    Key::F15,
    Key::F16,
    Key::F17,
    Key::F18,
    Key::F19,
    Key::F20,
    Key::F21,
    Key::F22,
    Key::F23,
    Key::F24,
    Key::F25,
    Key::KP0,
    Key::KP1,
    Key::KP2,
    Key::KP3,
    Key::KP4,
    Key::KP5,
    Key::KP6,
    Key::KP7,
    Key::KP8,
    Key::KP9,
    Key::KPDecimal,
    Key::KPDivide,
    Key::KPMultiply,
    Key::KPSubtract,
    Key::KPAdd,
    Key::KPEnter,
    Key::KPEqual,
    Key::LeftShift,
    Key::LeftControl,
    Key::LeftAlt,
    Key::LeftSuper,
    Key::RightShift,
    Key::RightControl,
    Key::RightAlt,
    Key::RightSuper,
    Key::Menu
};

std::unordered_set allMouseButtons = {
	MouseButton::MouseButton1,
	MouseButton::MouseButton2,
	MouseButton::MouseButton3,
	MouseButton::MouseButton4,
	MouseButton::MouseButton5,
	MouseButton::MouseButton6,
	MouseButton::MouseButton7,
	MouseButton::MouseButton8
};

namespace Coral {
    std::unordered_map<Key, KeyState> Input::m_keyboardKeyStates;
    std::unordered_map<MouseButton, KeyState> Input::m_mouseButtonStates;

    glm::ivec2 Input::m_mousePosition = { 0, 0 };
    glm::ivec2 Input::m_mouseDelta = { 0, 0 };
    glm::vec2 Input::m_scrollDelta = { 0.0f, 0.0f };

    void Input::Setup() {
        for (const auto key : allKeys) {
            m_keyboardKeyStates[key] = KeyState::NotPressed;
        }

        for (const auto button : allMouseButtons) {
            m_mouseButtonStates[button] = KeyState::NotPressed;
        }
    }

    void Input::Update() {
        for (const auto key : allKeys) {
            if (m_keyboardKeyStates[key] == KeyState::Pressed) {
                m_keyboardKeyStates[key] = KeyState::Held;
            } else if (m_keyboardKeyStates[key] == KeyState::Released) {
                m_keyboardKeyStates[key] = KeyState::NotPressed;
            }
        }

        for (const auto button : allMouseButtons) {
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

    glm::ivec2 Input::GetMousePosition() {
        return m_mousePosition;
    }

    glm::ivec2 Input::GetMousePositionDelta() {
        return m_mouseDelta;
    }

    glm::vec2 Input::GetMoseScrollDelta() {
        return m_scrollDelta;
    }

	void Input::Callbacks::keyCallback(GLFWwindow *, int key, int, const int action, int) {
    	const auto k = static_cast<Key>(key);
    	switch (action) {
    	case GLFW_PRESS:
    		Input::m_keyboardKeyStates[k] = KeyState::Pressed;
    		break;
    	case GLFW_RELEASE:
    		Input::m_keyboardKeyStates[k] = KeyState::Released;
    		break;
    	default:
    		break;
    	}
    }

	void Input::Callbacks::mouseMoveCallback(GLFWwindow *, const double x, const double y) {
    	const glm::ivec2 pos { static_cast<int>(x), static_cast<int>(y) };
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
    	m_scrollDelta = { x, y };
    }
}
