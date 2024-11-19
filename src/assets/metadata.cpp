//
// Created by radue on 11/5/2024.
//

#include "metadata.h"

#include <fstream>

#include <nlohmann/json.hpp>

namespace Asset {
    void Metadata::AddMeshMapping(const std::string &uuid, const uint32_t id) {
        meshMapping[uuid] = id;
        meshMappingInverse[id] = uuid;
    }

    uint32_t Metadata::GetMeshID(const std::string &uuid) const {
        if (!meshMapping.contains(uuid)) {
            return -1;
        }
        return meshMapping.at(uuid);
    }

    std::string Metadata::GetMeshUUID(const uint32_t id) const {
        if (!meshMappingInverse.contains(id)) {
            return "";
        }
        return meshMappingInverse.at(id);
    }

    void Metadata::AddMaterialMapping(const std::string &uuid, uint32_t id) {
        materialMapping[uuid] = id;
        materialMappingInverse[id] = uuid;
    }

    uint32_t Metadata::GetMaterialID(const std::string &uuid) const {
        if (!materialMapping.contains(uuid)) {
            return -1;
        }
        return materialMapping.at(uuid);
    }

    std::string Metadata::GetMaterialUUID(uint32_t id) const {
        if (!materialMappingInverse.contains(id)) {
            return "";
        }
        return materialMappingInverse.at(id);
    }

    std::unique_ptr<Metadata> Metadata::Load(const std::string &name) {
        const std::string metadataName = name + ".meta";
        const std::string path = "metadata/" + metadataName;
        std::ifstream file(path);
        if (!file.is_open()) {
            return nullptr;
        }
        nlohmann::json json;
        file >> json;
        auto metadata = std::make_unique<Metadata>(json["path"]);
        metadata->meshMapping = json["meshMapping"];
        for (const auto& [uuid, id] : metadata->meshMapping) {
            metadata->meshMappingInverse[id] = uuid;
        }
        metadata->materialMapping = json["materialMapping"];
        for (const auto& [uuid, id] : metadata->materialMapping) {
            metadata->materialMappingInverse[id] = uuid;
        }
        return metadata;
    }

    void Metadata::Save() const {
        std::string filename = m_path.substr(m_path.find_last_of("/\\") + 1);
        filename = filename.substr(0, filename.find_last_of("."));
        std::ofstream file("metadata/" + filename + ".meta");
        nlohmann::json json;
        json["path"] = m_path;
        json["meshMapping"] = meshMapping;
        json["materialMapping"] = materialMapping;
        file << json.dump(4);
    }
}
