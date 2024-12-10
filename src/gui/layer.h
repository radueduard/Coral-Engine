//
// Created by radue on 11/5/2024.
//

#pragma once

#include "manager.h"

namespace GUI {
    class Layer {
        friend class Manager;
    public:
        Layer() { Manager::AddLayer(this); }
        virtual ~Layer() { Manager::RemoveLayer(this); }

    private:
        virtual void OnUIAttach() = 0;
        virtual void OnUIUpdate() = 0;
        virtual void OnUIRender() = 0;
        virtual void OnUIReset() = 0;
        virtual void OnUIDetach() = 0;
    };

    class SubLayer {
    public:
        virtual ~SubLayer() = default;

    private:
        virtual void OnUIRender() = 0;
    };
}
