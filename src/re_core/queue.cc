module core.queue;
import core.device;

import std;

namespace Coral::Core {
	Queue::Queue(class Family& family): m_family(family) {
		if (m_family.m_remainingQueues == 0) {
			throw std::runtime_error("Queue::Queue : No more queues available in this family");
		}
		m_index = m_family.m_properties.queueCount - m_family.m_remainingQueues--;
		m_handle = GlobalDevice()->getQueue(m_family.Index(), m_index);
	}

	Queue::~Queue() {
		m_family.m_remainingQueues++;
	}
}