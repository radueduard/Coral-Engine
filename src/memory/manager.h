//
// Created by radue on 12/6/2024.
//

#pragma once

#include <memory>
#include <unordered_map>

#include <boost/unordered_map.hpp>
#include <boost/uuid/uuid.hpp>

namespace Memory {
    class Buffer;
}

namespace Memory {
    class Manager {
    public:
        static void Init() {
            m_instance = new Manager();
        }
        static void Destroy() {
            delete m_instance;
        }

        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;

        static void DrawUI();

    private:
        Manager() = default;
        ~Manager() = default;
        inline static Manager* m_instance = nullptr;

        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Buffer>> m_buffers;
    };
}
