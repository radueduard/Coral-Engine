//
// Created by radue on 10/17/2024.
//

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

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
}
