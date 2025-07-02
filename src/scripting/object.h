//
// Created by radue on 4/11/2025.
//

#pragma once
#include <iostream>
#include <mono/metadata/object.h>

#include "class.h"
#include "domain.h"
#include "utils/types.h"

namespace Coral::Scripting {
    class Object {
        friend class Class;
        inline static int s_instanceCount = 0;
    public:
        Object(const Class& klass, MonoObject* object) : m_class(klass), m_object(object) {
            gcHandle = mono_gchandle_new(m_object, true);
        }

        explicit Object(const Class& klass) : m_class(klass) {
            const auto domain = *Domain::Get();
            m_object = mono_object_new(domain, *m_class);
            gcHandle = mono_gchandle_new(m_object, true);
        }

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        ~Object() {
            if (m_object) {
                mono_gchandle_free(gcHandle);
            }
        }

        template<RemoteClass T, RemoteClass... Args>
        [[nodiscard]] T* Call(const std::string& methodName, Args*... args) const {
            const auto method = m_class.GetMethod(methodName);
            return method.Invoke(m_object, std::forward<Args*>(args)...);
        }

        MonoObject* operator*() const { return m_object; }

        const Class& GetClass() const {
            return m_class;
        }

    private:
        const Class& m_class;
        MonoObject* m_object;
        u32 gcHandle = 0;
    };
}
