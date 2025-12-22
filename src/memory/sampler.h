//
// Created by radue on 11/17/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "utils/globalWrapper.h"

namespace Coral::Core {
    class Device;
}

namespace Coral::Memory {

    class Sampler final : public EngineWrapper<vk::Sampler> {
    public:
    	class Builder {
    		friend class Sampler;
    	public:
    		Builder& MagFilter(vk::Filter filter) {
				magFilter = filter;
				return *this;
			}
			Builder& MinFilter(vk::Filter filter) {
				minFilter = filter;
				return *this;
			}
			Builder& AddressMode(vk::SamplerAddressMode mode) {
				addressMode = mode;
				return *this;
			}
			Builder& MipmapMode(vk::SamplerMipmapMode mode) {
				mipmapMode = mode;
				return *this;
			}

			std::unique_ptr<Sampler> Build() const {
				return std::make_unique<Sampler>(*this);
			}
    	private:
    		vk::Filter magFilter = vk::Filter::eLinear;
    		vk::Filter minFilter = vk::Filter::eLinear;
    		vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
    		vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
    	};

        explicit Sampler(const Builder& builder);
        ~Sampler() override;

        [[nodiscard]] vk::Filter MagFilter() const { return m_magFilter; }
        [[nodiscard]] vk::Filter MinFilter() const { return m_minFilter; }
        [[nodiscard]] vk::SamplerAddressMode AddressMode() const { return m_addressMode; }
        [[nodiscard]] vk::SamplerMipmapMode MipmapMode() const { return m_mipmapMode; }

    private:
        vk::Filter m_magFilter = vk::Filter::eLinear;
        vk::Filter m_minFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode m_addressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerMipmapMode m_mipmapMode = vk::SamplerMipmapMode::eLinear;
    };
}
