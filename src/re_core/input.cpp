//
// Created by radue on 4/18/2025.
//

export module input;

import <GLFW/glfw3.h>;

import std;
import math.vector;
import types;

namespace Coral {
	class Engine;
	namespace Core {
		class Window;
	}

	export enum class Key : u16 {
		Space = 32,
		Apostrophe = 39,
		Comma = 44,
		Minus = 45,
		Period = 46,
		Slash = 47,
		Num0 = 48,
		Num1 = 49,
		Num2 = 50,
		Num3 = 51,
		Num4 = 52,
		Num5 = 53,
		Num6 = 54,
		Num7 = 55,
		Num8 = 56,
		Num9 = 57,
		Semicolon = 59,
		Equal = 61,
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LeftBracket = 91,
		Backslash = 92,
		RightBracket = 93,
		GraveAccent = 96,
		World1 = 161,
		World2 = 162,
		Esc = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	};

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

	export enum class MouseButton : u8 {
		MouseButton1 = 0,
		MouseButton2 = 1,
		MouseButton3 = 2,
		MouseButton4 = 3,
		MouseButton5 = 4,
		MouseButton6 = 5,
		MouseButton7 = 6,
		MouseButton8 = 7,
		MouseButtonLeft = MouseButton1,
		MouseButtonRight = MouseButton2,
		MouseButtonMiddle = MouseButton3
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

	export enum class SpecialKey : u8 {
		Shift = 1 << 0,
		Ctrl = 1 << 1,
		Alt = 1 << 2,
		Super = 1 << 3,
		CapsLock = 1 << 4,
		NumLock = 1 << 5,
	};

	export enum class KeyState : u8 {
		NotPressed,
		Pressed,
		Held,
		Released,
	};

	export class Input {
        friend class Engine;
    	friend class Core::Window;
    public:
		static KeyState GetKeyState(const Key key) {
			return m_keyboardKeyStates[key];
		}

		static KeyState GetMouseButtonState(const MouseButton button) {
			return m_mouseButtonStates[button];
		}

		static bool IsKeyPressed(const Key key) {
			return m_keyboardKeyStates[key] == KeyState::Pressed;
		}

		static bool IsKeyHeld(const Key key) {
			return m_keyboardKeyStates[key] == KeyState::Held;
		}

		static bool IsKeyReleased(const Key key) {
			return m_keyboardKeyStates[key] == KeyState::Released;
		}

		static bool IsMouseButtonPressed(const MouseButton button) {
			return m_mouseButtonStates[button] == KeyState::Pressed;
		}

		static bool IsMouseButtonHeld(const MouseButton button) {
			return m_mouseButtonStates[button] == KeyState::Held;
		}

		static bool IsMouseButtonReleased(const MouseButton button) {
			return m_mouseButtonStates[button] == KeyState::Released;
		}

		static const Math::Vector2<f64>& GetMousePosition() {
			return m_mousePosition;
		}

		static const Math::Vector2<f64>& GetMousePositionDelta() {
			return m_mouseDelta;
		}

		static const Math::Vector2<f64>& GetMoseScrollDelta() {
			return m_scrollDelta;
		}

    private:
		static void Setup() {
			for (const auto key : allKeys) {
				m_keyboardKeyStates[key] = KeyState::NotPressed;
			}

			for (const auto button : allMouseButtons) {
				m_mouseButtonStates[button] = KeyState::NotPressed;
			}
		}

		static void Update() {
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

			m_mouseDelta = { 0.0, 0.0 };
			m_scrollDelta = { 0.0, 0.0 };
		}

        static std::unordered_map<Key, KeyState> m_keyboardKeyStates;
        static std::unordered_map<MouseButton, KeyState> m_mouseButtonStates;

        static Math::Vector2<f64> m_mousePosition;
        static Math::Vector2<f64> m_mouseDelta;
        static Math::Vector2<f64> m_scrollDelta;

    	struct Callbacks {
    		static void keyCallback(GLFWwindow *, const i32 key, int, const i32 action, i32) {
    			const auto k = static_cast<const Key>(key);
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

    		static void mouseMoveCallback(GLFWwindow *, const f64 x, const f64 y) {
    			const Math::Vector2 pos { x, y };
    			m_mouseDelta = pos - m_mousePosition;
    			m_mousePosition = pos;

    		}

    		static void mouseButtonCallback(GLFWwindow*, const i32 button, const i32 action, int) {
    			const auto b = static_cast<const MouseButton>(button);
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

    		static void scrollCallback(GLFWwindow*, const f64 x, const f64 y) {
    			m_scrollDelta = { x, y };
    		}
    	};
    };

	std::unordered_map<Key, KeyState> Input::m_keyboardKeyStates;
	std::unordered_map<MouseButton, KeyState> Input::m_mouseButtonStates;

	Math::Vector2<f64> Input::m_mousePosition = { 0.0, 0.0 };
	Math::Vector2<f64> Input::m_mouseDelta = { 0.0, 0.0 };
	Math::Vector2<f64> Input::m_scrollDelta = { 0.0, 0.0 };
}
