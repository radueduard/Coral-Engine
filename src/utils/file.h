//
// Created by radue on 10/17/2024.
//

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <sstream>

#include <stb_image.h>

#include <glm/glm.hpp>

namespace Utils {
    inline std::vector<uint8_t> ReadBinaryFile(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        std::vector<uint8_t> buffer;
        file.seekg(0, std::ios::end);
        buffer.resize(file.tellg());
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        file.close();

        return buffer;
    }

    inline std::string ReadTextFile(const std::filesystem::path &path) {
        const std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    inline void WriteBinaryFile(const std::string &path, const std::vector<uint8_t> &data) {
        std::ofstream file(path, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
    }

    inline void WriteTextFile(const std::string &path, const std::string &data) {
        std::ofstream file(path, std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        file << data;
        file.close();
    }

    inline void ReadImageData(const std::filesystem::path &path, std::vector<glm::u8vec4> &data, int &width, int &height, int &channels) {
        FILE* file;
        fopen_s(&file, path.string().c_str(), "rb");
        if (!file) {
            throw std::runtime_error("Failed to open image file: " + path.string());
        }
        unsigned char* imageData = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
        if (!imageData) {
            fclose(file);
            throw std::runtime_error("Failed to load image data: " + path.string());
        }
        data.resize(width * height);
        for (int i = 0; i < width * height; ++i) {
            data[i] = glm::u8vec4(imageData[i * channels], imageData[i * channels + 1], imageData[i * channels + 2], (channels == 4) ? imageData[i * channels + 3] : 255);
        }
        stbi_image_free(imageData);
        fclose(file);
    }

    inline std::unordered_map<std::string, std::string> FileTypes = {
        {"vert", "Vertex Shader"},
        {"tesc", "Tessellation Control Shader"},
        {"tese", "Tessellation Evaluation Shader"},
        {"geom", "Geometry Shader"},
        {"frag", "Fragment Shader"},
        {"comp", "Compute Shader"},
        {"task", "Task Shader"},
        {"mesh", "Mesh Shader"},
        {"spv", "SPIR-V"},
        {"txt", "Text File"},
        {"bin", "Binary File"},
    };

    inline std::unordered_map<std::string, std::string> IconPaths = {
        { "Vertex Shader", "assets/icons/icon_vert.png" },
        { "Tessellation Control Shader", "assets/icons/icon_tesc.png" },
        { "Tessellation Evaluation Shader", "assets/icons/icon_tese.png" },
        { "Geometry Shader", "assets/icons/icon_geom.png" },
        { "Fragment Shader", "assets/icons/icon_frag.png" },
        { "Compute Shader", "assets/icons/icon_comp.png" },
        { "Task Shader", "assets/icons/icon_task.png" },
        { "Mesh Shader", "assets/icons/icon_mesh.png" },
        { "SPIR-V", "assets/icons/icon_spv.png" },
        { "Text File", "assets/icons/icon_text.png" },
        { "Binary File", "assets/icons/icon_bin.png" },
        { "Directory", "assets/icons/icon_dir.png" },
    };
}
