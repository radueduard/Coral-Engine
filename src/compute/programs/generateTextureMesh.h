//
// Created by radue on 1/3/2026.
//

#pragma once
#include "memory/image.h"

namespace Coral::Graphics {
	class Mesh;
}
namespace Coral::Compute {
	class GenerateTextureMesh {
	public:
		explicit GenerateTextureMesh(const Memory::Image& image, const Math::Vector3u& groupCount);
		~GenerateTextureMesh() = default;

		std::unique_ptr<Graphics::Mesh> Execute(const Math::Vector3u& chunkID, const Math::Vector3u& chunkCount) const;

	private:
		const Memory::Image& m_image;

		std::unique_ptr<Memory::Buffer> m_vertexBuffer;
		std::unique_ptr<Memory::Buffer> m_indexBuffer;

		std::unique_ptr<Memory::Buffer> m_vertexCountBuffer;
		std::unique_ptr<Memory::Buffer> m_indexCountBuffer;

		u32 m_vertexCount = 0;
		u32 m_indexCount = 0;

		Math::Vector3u m_groupCount;
	};
}
