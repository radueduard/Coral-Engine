//
// Created by radue on 11/29/2024.
//

#pragma once
#include <memory>

#include "compute/pipeline.h"

namespace Memory::Descriptor {
    class SetLayout;
}

namespace Compute {
    class Program {
    public:
        Program() = default;
        virtual ~Program() = default;

        Program(const Program &) = delete;
        Program &operator=(const Program &) = delete;

        virtual void Init() = 0;
        virtual void Update() = 0;
        virtual void Compute() = 0;
        virtual void ResetDescriptorSets() = 0;

    protected:
        std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
        std::unique_ptr<Pipeline> m_pipeline;
    };
}
