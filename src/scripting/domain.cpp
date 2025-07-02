//
// Created by radue on 4/11/2025.
//

#include "domain.h"

#include <iostream>
#include <stdexcept>

namespace Coral::Scripting {
    Domain::Domain(std::string name, std::string configFile) {
        m_domain = mono_domain_create_appdomain(name.data(), configFile.empty() ? nullptr : configFile.data());
    }

    Domain::~Domain() {
        if (this != s_rootDomain) {
            try {
                s_rootDomain->Set();
                mono_domain_free(m_domain, true);
            } catch (const std::exception& e) {
                std::cerr << "Failed to free domain: " << e.what() << std::endl;
            }
        } else {
            mono_jit_cleanup(m_domain);
        }
    }

    void Domain::Set() const {
        mono_domain_set(m_domain, true);
        s_setDomain = this;
    }

    const Domain& Domain::Get() {
        return *s_setDomain;
    }

    void Domain::CreateRoot() {
        if (s_rootDomain == nullptr) {
            const auto domain = mono_jit_init("RootDomain");
            s_rootDomain = new Domain(domain);
            s_rootDomain->Set();
        }
    }

    void Domain::DestroyRoot() {
        if (s_rootDomain) {
            delete s_rootDomain;
            s_rootDomain = nullptr;
        }
    }

    Domain::Domain(MonoDomain* domain) : m_domain(domain) {}

    const Domain& Domain::Root() {
        if (!s_rootDomain) {
            throw std::runtime_error("Root domain is not initialized");
        }
        return *s_rootDomain;
    }

}
