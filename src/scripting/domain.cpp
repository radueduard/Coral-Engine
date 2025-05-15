//
// Created by radue on 4/11/2025.
//

#include "domain.h"

#include <stdexcept>

namespace Coral::Scripting {
    Domain::Domain(std::string name, std::string configFile) {
        m_domain = mono_domain_create_appdomain(name.data(), configFile.empty() ? nullptr : configFile.data());
    }

    Domain::~Domain() {
        if (m_domain && m_domain != s_rootDomain->m_domain) {
            mono_domain_free(m_domain, true);
        }
    }

    void Domain::Set() const {
        mono_domain_set(m_domain, true);
        s_setDomain = this;
    }

    const Domain& Domain::Get() {
        return *s_setDomain;
    }


    Domain::Domain(const std::string& fileName) {
        if (!s_rootDomain) {
            s_rootDomain = this;
        } else {
            throw std::runtime_error("Root domain already exists");
        }

        m_domain = mono_jit_init(fileName.c_str());
        s_setDomain = this;
    }

    void Domain::JitCleanup::operator()(const Domain* domain) const {
        mono_jit_cleanup(domain->m_domain);
    }

    const Domain& Domain::Root() {
        if (!s_rootDomain) {
            throw std::runtime_error("Root domain is not initialized");
        }
        return *s_rootDomain;
    }

}
