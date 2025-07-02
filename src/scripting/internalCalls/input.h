//
// Created by radue on 6/13/2025.
//

#pragma once
#include <mono/metadata/object-forward.h>

#include "core/input.h"

namespace Coral::Scripting {
	class Input {
	public:
		static MonoObject* GetMousePosition() {
			auto& mousePosition = Coral::Input::GetMousePosition();
			const auto* remoteInstance = mousePosition.RemoteInstance();
			return **remoteInstance;
		}

		static MonoObject* GetMousePositionDelta() {
			auto& mousePositionDelta = Coral::Input::GetMousePositionDelta();
			const auto* remoteInstance = mousePositionDelta.RemoteInstance();
			return **remoteInstance;
		}
		static MonoObject* GetMouseScrollDelta() {
			auto& mouseScrollDelta = Coral::Input::GetMouseScrollDelta();
			const auto* remoteInstance = mouseScrollDelta.RemoteInstance();
			return **remoteInstance;
		}
	};
}