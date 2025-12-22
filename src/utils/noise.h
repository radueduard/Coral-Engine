//
// Created by radue on 12/20/2025.
//

#pragma once

#include "memory/buffer.h"
#include "memory/image.h"
#include "random.h"

namespace Coral::Utils {
	class Noise {
	public:
		Memory::Image& Image() const {
			return *m_image;
		}
	protected:
		std::unique_ptr<Memory::Image> m_image = nullptr;
	};

	template<u8 N>
	class CircleNoise : public Noise {
	public:
		explicit CircleNoise(Math::Vector<u32, N> size, const f32 radius, const u32 octaves = 1)
			: m_size(size), m_radius(radius) {

			m_image = Memory::Image::Builder()
				.Format(vk::Format::eR8Unorm)
				.Extent(size)
				.MipLevels(octaves)
				.UsageFlags(vk::ImageUsageFlagBits::eSampled)
				.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
				.InitialLayout(vk::ImageLayout::eTransferDstOptimal)
				.Build();

			u32 valueCount = 1;
			for (u8 i = 0; i < N; i++) {
				valueCount *= m_size[i];
			}

			for (int octave = 0; octave < octaves; octave++) {
				std::vector<u8> data(valueCount, 0);
				if constexpr (N == 2) {
					for (u32 y = 0; y < size[1]; y++) {
						for (u32 x = 0; x < size[0]; x++) {
							Math::Vector2f uv = {
								static_cast<f32>(x) / static_cast<f32>(size[0] - 1),
								static_cast<f32>(y) / static_cast<f32>(size[1] - 1)
							};
							uv = uv * 2.f - Math::Vector2f{ 1.f, 1.f };
							const f32 dist = uv.Length() * (static_cast<f32>(size[0]) / 2.f);
							const f32 value = dist < radius ? 1.f : 0.f;
							data[y * size[0] + x] = static_cast<u8>(value * 255.f);
						}
					}
				}
				else if constexpr (N == 3) {
					for (u32 z = 0; z < size[2]; z++) {
						for (u32 y = 0; y < size[1]; y++) {
							for (u32 x = 0; x < size[0]; x++) {
								Math::Vector3f uvw = {
									static_cast<f32>(x) / static_cast<f32>(size[0] - 1),
									static_cast<f32>(y) / static_cast<f32>(size[1] - 1),
									static_cast<f32>(z) / static_cast<f32>(size[2] - 1)
								};
								uvw = uvw * 2.f - Math::Vector3f{ 1.f, 1.f, 1.f };
								const f32 dist = uvw.Length();
								const f32 value = dist < radius ? 1.f : 0.f;
								data[z * size[0] * size[1] + y * size[0] + x] = static_cast<u8>(value * 255.f);
							}
						}
					}
				}

				const auto stagingBuffer = Memory::Buffer::Builder()
					.InstanceSize(sizeof(u8))
					.InstanceCount(data.size())
					.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
					.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
					.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
					.Build();
				stagingBuffer->Map<u8>();
				stagingBuffer->Write(std::span(data.data(), data.size()));
				stagingBuffer->Flush();
				stagingBuffer->Unmap();

				m_image->Copy(*stagingBuffer, octave);
				valueCount /= std::pow(2, N);
				size /= 2;
			}
			m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}
	private:
		Math::Vector<u32, N> m_size;
		f32 m_radius;
	};

	template<u8 N>
	class WhiteNoise : public Noise {
	public:
		explicit WhiteNoise(Math::Vector<u32, N> size, u32 octaves, const f32 minValue = 0.f, const f32 maxValue = 1.f)
			: m_size(size), m_minValue(minValue), m_maxValue(maxValue) {

			m_image = Memory::Image::Builder()
				.Format(vk::Format::eR8Unorm)
				.Extent(size)
				.MipLevels(octaves)
				.UsageFlags(vk::ImageUsageFlagBits::eSampled)
				.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
				.InitialLayout(vk::ImageLayout::eTransferDstOptimal)
				.Build();

			u32 valueCount = 1;
			for (u8 i = 0; i < N; i++) {
				valueCount *= m_size[i];
			}

			for (u8 i = 0; i < octaves; i++) {
				std::vector<u8> data(valueCount, 0);
				for (unsigned char & val : data) {
					val = Random::UniformIntegralValue<u16>(0, 255);
				}

				const auto stagingBuffer = Memory::Buffer::Builder()
					.InstanceSize(sizeof(u8))
					.InstanceCount(data.size())
					.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
					.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
					.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
					.Build();

				stagingBuffer->Map<u8>();
				stagingBuffer->Write(std::span(data.data(), data.size()));
				stagingBuffer->Flush();
				stagingBuffer->Unmap();

				m_image->Copy(*stagingBuffer, i);

				valueCount /= std::pow(2, N);
			}

			m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}



	private:
		Math::Vector<u32, N> m_size;
		f32 m_minValue;
		f32 m_maxValue;
	};

	template<u8 DIM, u8 N>
	class DirectionsNoise : public Noise {
	public:
		explicit DirectionsNoise(Math::Vector<u32, N> size, u32 octaves = 1)
			: m_size(size) {

			constexpr vk::Format format =
				(DIM == 1) ? vk::Format::eR8Unorm :
				(DIM == 2) ? vk::Format::eR8G8Unorm :
				(DIM == 3) ? vk::Format::eR8G8B8A8Unorm :
				throw std::runtime_error("DirectionsNoise : DIM must be 1, 2 or 3");

			m_image = Memory::Image::Builder()
				.Format(format)
				.Extent(size)
				.MipLevels(octaves)
				.UsageFlags(vk::ImageUsageFlagBits::eSampled)
				.UsageFlags(vk::ImageUsageFlagBits::eStorage)
				.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
				.InitialLayout(vk::ImageLayout::eTransferDstOptimal)
				.Build();

			u32 valueCount = 1;
			for (u8 i = 0; i < N; i++) {
				valueCount *= m_size[i];
			}

			for (u8 i = 0; i < octaves; i++) {
				std::vector<u8> data(valueCount * 4, 0);
				for (u32 j = 0; j < valueCount; j++) {
					Math::Vector3f dir = Random::NormalVector<3, f32>(
						Math::Vector3f(0.f, 0.f, 0.f),
						Math::Vector3f(1.f, 1.f, 1.f)
					).Normalized();
					data[j * 4 + 0] = static_cast<u8>((dir.x * 0.5f + 0.5f) * 255.f);
					data[j * 4 + 1] = static_cast<u8>((dir.y * 0.5f + 0.5f) * 255.f);
					data[j * 4 + 2] = static_cast<u8>((dir.z * 0.5f + 0.5f) * 255.f);
					data[j * 4 + 3] = 255;
				}

				const auto stagingBuffer = Memory::Buffer::Builder()
					.InstanceSize(sizeof(u8))
					.InstanceCount(data.size())
					.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
					.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
					.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
					.Build();

				stagingBuffer->Map<u8>();
				stagingBuffer->Write(std::span(data.data(), data.size()));
				stagingBuffer->Flush();
				stagingBuffer->Unmap();

				m_image->Copy(*stagingBuffer, i);
				valueCount /= std::pow(2, N);
			}
			m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}
	private:
		Math::Vector<u32, N> m_size;
	};

	class PerlinNoise3D : public Noise {
	public:
		explicit PerlinNoise3D(Math::Vector3u size, u32 octaves);

	private:
		Math::Vector3u m_size;
	};
}