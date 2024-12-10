//
// Created by radue on 11/15/2024.
//

#include "program.h"

#include "graphics/renderPass.h"

namespace Graphics {
    Program::Program(const std::vector<RenderPass*>& renderPasses): m_renderPasses(renderPasses) {
        for (const auto renderPass: m_renderPasses) {
            renderPass->AddProgram(this);
            m_pipelines[renderPass] = nullptr;
        }
    }

    Program::~Program() {
        for (const auto renderPass: m_renderPasses) {
            renderPass->RemoveProgram(this);
        }
    }
}
