//
// Created by radue on 1/1/2026.
//

#pragma once
#include <memory>

#include "compute/pipeline.h"
#include "memory/buffer.h"
#include "shader/manager.h"

namespace Coral::Compute {
	class BitonicMergeSorter {
	public:
		explicit BitonicMergeSorter(const std::vector<u32>& elements);
		~BitonicMergeSorter() = default;

		std::vector<u32> operator ()() const;

	private:
		std::unique_ptr<Memory::Buffer> m_buffer = nullptr;
		std::unique_ptr<Pipeline> m_pipeline = nullptr;
	};
}
