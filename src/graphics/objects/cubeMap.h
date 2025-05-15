//
// Created by radue on 11/29/2024.
//

#pragma once
#include <memory>
#include <string>

#include <vulkan/vulkan.hpp>

#include "memory/sampler.h"
#include "memory/image.h"
#include "memory/imageView.h"

namespace Coral::Core {
    class Device;
}

namespace Coral::Graphics {
    class CubeMap {
    public:
        class Builder {
            friend class CubeMap;
        public:
            Builder() = default;
            Builder& PositiveX(const std::string& path) {
                m_positiveX = path;
                return *this;
            }

            Builder& NegativeX(const std::string& path) {
                m_negativeX = path;
                return *this;
            }

            Builder& PositiveY(const std::string& path) {
                m_positiveY = path;
                return *this;
            }

            Builder& NegativeY(const std::string& path) {
                m_negativeY = path;
                return *this;
            }

            Builder& PositiveZ(const std::string& path) {
                m_positiveZ = path;
                return *this;
            }

            Builder& NegativeZ(const std::string& path) {
                m_negativeZ = path;
                return *this;
            }

            [[nodiscard]] std::unique_ptr<CubeMap> Build() const {
                if (m_positiveX.empty() || m_negativeX.empty() || m_positiveY.empty() || m_negativeY.empty() || m_positiveZ.empty() || m_negativeZ.empty()) {
                    throw std::runtime_error("CubeMap images not provided");
                }
                return std::make_unique<CubeMap>(*this);
            }
        private:
            std::string m_positiveX;
            std::string m_negativeX;
            std::string m_positiveY;
            std::string m_negativeY;
            std::string m_positiveZ;
            std::string m_negativeZ;
        };

        explicit CubeMap(const Builder& builder);
        ~CubeMap() = default;

        CubeMap(const CubeMap&) = delete;
        CubeMap& operator=(const CubeMap&) = delete;

        [[nodiscard]] const Memory::Image* Image() const { return m_image.get(); }
        [[nodiscard]] uint32_t Size() const { return m_size; }
        [[nodiscard]] vk::DescriptorImageInfo DescriptorInfo() const;

    private:
        uint32_t m_size;
        std::unique_ptr<Memory::Image> m_image;
        std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;
        std::unique_ptr<Memory::Sampler> m_sampler;
    };
}
