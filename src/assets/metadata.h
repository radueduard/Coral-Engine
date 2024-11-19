//
// Created by radue on 11/5/2024.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Asset {
    class Metadata {
    public:
        explicit Metadata(std::string  path) : m_path(std::move(path)) {}
        ~Metadata() = default;

        void AddMeshMapping(const std::string& uuid, uint32_t id);
        [[nodiscard]] uint32_t GetMeshID(const std::string& uuid) const;
        [[nodiscard]] std::string GetMeshUUID(uint32_t id) const;

        void AddMaterialMapping(const std::string& uuid, uint32_t id);
        [[nodiscard]] uint32_t GetMaterialID(const std::string& uuid) const;
        [[nodiscard]] std::string GetMaterialUUID(uint32_t id) const;

        static std::unique_ptr<Metadata> Create(std::string path) {
            return std::make_unique<Metadata>(std::move(path));
        }

        // TODO: Make it dependent on the scene
        static std::unique_ptr<Metadata> Load(const std::string &name);

        void Save() const;

    private:
        std::string m_path;
        std::unordered_map<std::string, uint32_t> meshMapping;
        std::unordered_map<uint32_t, std::string> meshMappingInverse;

        std::unordered_map<std::string, uint32_t> materialMapping;
        std::unordered_map<uint32_t, std::string> materialMappingInverse;
    };
}
