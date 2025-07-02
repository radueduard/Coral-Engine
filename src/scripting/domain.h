//
// Created by radue on 4/11/2025.
//

#pragma once

#include <string>

#include <mono/jit/jit.h>

namespace Coral::Scripting {
    class Domain;

    static Domain* s_rootDomain = nullptr;

    class Domain {
    public:
        MonoDomain* operator*() const { return m_domain; }

        explicit Domain(std::string name, std::string configFile);
        ~Domain();

        void Set() const;
        static const Domain& Get();

        Domain(const Domain&) = delete;
        Domain& operator=(const Domain&) = delete;

        static const Domain& Root();
        static void CreateRoot();
        static void DestroyRoot();

    private:
        explicit Domain(MonoDomain* domain);
        inline static const Domain* s_rootDomain = nullptr;
        inline static const Domain* s_setDomain = nullptr;

        MonoDomain* m_domain;
    };



}
