//
// Created by radue on 4/11/2025.
//

#pragma once
#include <memory>
#include <stdexcept>
#include <string>

#include <mono/jit/jit.h>

class Engine;

namespace Coral::Scripting {
    class Domain;

    static Domain* s_rootDomain = nullptr;

    class Domain {
        struct JitCleanup {
            void operator()(const Domain* domain) const;
        };

        friend class ::Engine;
        friend struct JitCleanup;
    public:
        static const Domain& Root();
        const MonoDomain* operator*() const { return m_domain; }

        explicit Domain(std::string name, std::string configFile);
        ~Domain();

        void Set() const;
        static const Domain& Get();

        Domain(const Domain&) = delete;
        Domain& operator=(const Domain&) = delete;

    private:
        explicit Domain(const std::string& fileName);
        inline static const Domain* s_rootDomain = nullptr;
        inline static const Domain* s_setDomain = nullptr;

        MonoDomain* m_domain;
    };



}
