//
// Created by radue on 11/29/2024.
//

#include "program.h"

#include "graphics/renderer.h"

Compute::Program::Program() {
    mgv::Renderer::AddComputeProgram(this);
}

Compute::Program::~Program() {
    mgv::Renderer::RemoveComputeProgram(this);
}
