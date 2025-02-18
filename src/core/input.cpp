//
// Created by radue on 10/22/2024.
//

#include "input.h"

#include <unordered_set>

std::unordered_set allKeys = {
    Space,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Semicolon,
    Equal,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LeftBracket,
    Backslash,
    RightBracket,
    GraveAccent,
    World1,
    World2,
    Esc,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,
    KP0,
    KP1,
    KP2,
    KP3,
    KP4,
    KP5,
    KP6,
    KP7,
    KP8,
    KP9,
    KPDecimal,
    KPDivide,
    KPMultiply,
    KPSubtract,
    KPAdd,
    KPEnter,
    KPEqual,
    LeftShift,
    LeftControl,
    LeftAlt,
    LeftSuper,
    RightShift,
    RightControl,
    RightAlt,
    RightSuper,
    Menu
};

std::unordered_set allMouseButtons = {
    MouseButton1,
    MouseButton2,
    MouseButton3,
    MouseButton4,
    MouseButton5,
    MouseButton6,
    MouseButton7,
    MouseButton8
};

namespace Core {
    std::unordered_map<Key, KeyState> Input::m_keyboardKeyStates;
    std::unordered_map<MouseButton, KeyState> Input::m_mouseButtonStates;

    glm::ivec2 Input::m_mousePosition = { 0, 0 };
    glm::ivec2 Input::m_mouseDelta = { 0, 0 };
    glm::vec2 Input::m_scrollDelta = { 0.0f, 0.0f };

    void Input::Setup() {
        for (const auto key : allKeys) {
            m_keyboardKeyStates[key] = NotPressed;
        }

        for (const auto button : allMouseButtons) {
            m_mouseButtonStates[button] = NotPressed;
        }
    }

    void Input::Update() {
        for (const auto key : allKeys) {
            if (m_keyboardKeyStates[key] == Pressed) {
                m_keyboardKeyStates[key] = Held;
            } else if (m_keyboardKeyStates[key] == Released) {
                m_keyboardKeyStates[key] = NotPressed;
            }
        }

        for (const auto button : allMouseButtons) {
            if (m_mouseButtonStates[button] == Pressed) {
                m_mouseButtonStates[button] = Held;
            } else if (m_mouseButtonStates[button] == Released) {
                m_mouseButtonStates[button] = NotPressed;
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
        return m_keyboardKeyStates[key] == Pressed;
    }

    bool Input::IsKeyHeld(const Key key) {
        return m_keyboardKeyStates[key] == Held;
    }

    bool Input::IsKeyReleased(const Key key) {
        return m_keyboardKeyStates[key] == Released;
    }

    bool Input::IsMouseButtonPressed(const MouseButton button) {
        return m_mouseButtonStates[button] == Pressed;
    }

    bool Input::IsMouseButtonHeld(const MouseButton button) {
        return m_mouseButtonStates[button] == Held;
    }

    bool Input::IsMouseButtonReleased(const MouseButton button) {
        return m_mouseButtonStates[button] == Released;
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
}
