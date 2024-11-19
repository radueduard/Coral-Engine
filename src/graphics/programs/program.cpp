//
// Created by radue on 11/15/2024.
//

#include "program.h"

#include "graphics/renderPass.h"

namespace Graphics {
    Program::Program(RenderPass &renderPass, const uint32_t subpassIndex): m_renderPass(renderPass), m_subpassIndex(subpassIndex) {
        m_renderPass.AddProgram(this, subpassIndex);
    }

    Program::~Program() {
        m_renderPass.RemoveProgram(this);
    }
}
