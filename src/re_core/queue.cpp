//
// Created by radue on 4/18/2025.
//
export module core.queue;

import <vulkan/vulkan.hpp>;

import types;
import utils.wrapper;
import std;

namespace Coral::Core {
	export class Queue final : public Utils::Wrapper<vk::Queue> {
	public:
		class Family {
			friend class Queue;
		public:
			explicit Family(const u32 index, const vk::QueueFamilyProperties &properties, const bool canPresent)
				: m_index(index), m_properties(properties), m_canPresent(canPresent)
			{
				m_remainingQueues = properties.queueCount;
			}

			~Family() = default;

			[[nodiscard]] u32 Index() const { return m_index; }
			[[nodiscard]] const vk::QueueFamilyProperties& Properties() const { return m_properties; }
			[[nodiscard]] bool CanPresent() const { return m_canPresent; }

			[[nodiscard]] std::unique_ptr<Queue> RequestQueue() {
				try {
					return std::make_unique<Queue>(*this);
				} catch (const std::runtime_error& err) {
					throw std::runtime_error("QueueFamily::RequestQueue : \n" + std::string(err.what()));
				}
			}

			[[nodiscard]] std::unique_ptr<Queue> RequestPresentQueue() {
				if (m_canPresent) {
					return std::make_unique<Queue>(*this);
				}
				throw std::runtime_error("QueueFamily::RequestPresentQueue : Queue family cannot present");
			}

		private:
			u32 m_index;
			vk::QueueFamilyProperties m_properties;
			u32 m_remainingQueues = 0;
			bool m_canPresent = false;
		};

		explicit Queue(Family &family);
		~Queue() override;

		[[nodiscard]] u32 Index() const { return m_index; }
		[[nodiscard]] const Family& Family() const { return m_family; }

	private:
		u32 m_index;
		class Family& m_family;
	};
}
