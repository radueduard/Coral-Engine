//
// Created by radue on 1/15/2026.
//

#pragma once
#include <chrono>

namespace Coral {
	class Time {
	public:
		friend class Engine;

		template <typename T = float> requires std::is_floating_point_v<T>
		static T FrameTime() { return static_cast<T>(frameDeltaTime.count()); }

	private:
		static void Setup() {
			appStart = std::chrono::system_clock::now();
			lastFrameStart = appStart;
			lastFixedPoint = appStart;
			shouldRunFixedUpdate = false;
		}

		static void Update() {
			const auto now = std::chrono::system_clock::now();
			frameDeltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastFrameStart);
			lastFrameStart = now;

			if (now - lastFixedPoint >= fixedDeltaTime) {
				shouldRunFixedUpdate = true;
				lastFixedPoint = now;
			} else {
				shouldRunFixedUpdate = false;
			}
		}
		inline static std::chrono::time_point<std::chrono::system_clock> appStart;
		inline static std::chrono::time_point<std::chrono::system_clock> lastFrameStart;
		inline static std::chrono::duration<double> frameDeltaTime;

		inline static std::chrono::time_point<std::chrono::system_clock> lastFixedPoint;
		static constexpr auto fixedDeltaTime = std::chrono::duration<double>(1.0 / 60.0);
		inline static bool shouldRunFixedUpdate = false;

	};
}
