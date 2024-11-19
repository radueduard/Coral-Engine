//
// Created by radue on 11/5/2024.
//

#pragma once
#include "manager.h"

namespace GUI {

    class Layer {
    public:
        Layer() { Manager::AddLayer(this); }
        virtual ~Layer() { Manager::RemoveLayer(this); }

        virtual void InitUI() = 0;
        virtual void UpdateUI() = 0;
        virtual void DrawUI() = 0;
        virtual void DestroyUI() = 0;
    };

}
