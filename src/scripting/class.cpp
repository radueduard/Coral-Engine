//
// Created by radue on 4/22/2025.
//

#include "scripting/class.h"

#include <iostream>
#include <ranges>

#include "scripting/object.h"
#include "scripting/remote.h"

namespace Coral::Scripting {
    ClassField::ClassField(MonoClass* klass, const std::string& name) {
        m_field = mono_class_get_field_from_name(klass, name.c_str());
        if (m_field == nullptr) {
            throw std::runtime_error("Field not found: " + name);
        }
        m_type = mono_field_get_type(m_field);
        if (m_type == nullptr) {
            throw std::runtime_error("Field type not found: " + name);
        }
    }

    Method::Method(const Scripting::Class& klass, MonoMethod* method) : m_class(klass), m_method(method) {
        m_name = mono_method_get_name(m_method);
        if (m_method == nullptr) {
            throw std::runtime_error("Method not found: " + m_name);
        }

        MonoMethodSignature* signature = mono_method_signature(m_method);
        const uint32_t paramCount = mono_signature_get_param_count(signature);
        m_parameters.reserve(paramCount);

        void* iter = nullptr;
        for (int i = 0; i < paramCount; ++i) {
            MonoType* paramType = mono_signature_get_params(signature, &iter);
            m_parameters.emplace_back(paramType);
        }
    }

    Object* Class::CreateInstance() const {
        return new Object(*this);
    }

    void Class::PrintInfo() const {
        std::cout << "Class: " << mono_class_get_name(m_class) << std::endl;
        std::cout << "Namespace: " << mono_class_get_namespace(m_class) << std::endl;

        std::cout << std::endl;

        std::cout << "Fields:" << std::endl;
        for (const auto& [name, field] : m_fields) {
            std::cout << "Field: " << name << ", Type: " << mono_type_get_name(field->Type()) << std::endl;
        }

        std::cout << std::endl;

        std::cout << "Methods:" << std::endl;
        for (const auto& [name, method] : m_methods) {
            std::cout << "Method: " << name << ", Return Type: " << mono_type_get_name(method->ReturnType()) << std::endl;
            std::cout << "Parameters:" << std::endl;
            for (const auto& paramType : method->Parameters()) {
                std::cout << "  - " << mono_type_get_name(paramType) << std::endl;
            }
        }
    }

    Class::Class(MonoImage* image, const std::string& namespac, const std::string& name) {
        m_class = mono_class_from_name(image, namespac.c_str(), name.c_str());
        if (m_class == nullptr) {
            throw std::runtime_error("Class not found: " + namespac + "." + name);
        }

        // Get fields
        void* iter = nullptr;
        MonoClassField* field = nullptr;
        while ((field = mono_class_get_fields(m_class, &iter))) {
            const char* fieldName = mono_field_get_name(field);
            if (fieldName == nullptr) {
                throw std::runtime_error("Field name not found");
            }
            m_fields[fieldName] = new ClassField(m_class, fieldName);
        }

        // Get methods
        iter = nullptr;
        MonoMethod* method = nullptr;
        while ((method = mono_class_get_methods(m_class, &iter))) {
            const char* methodName = mono_method_get_name(method);
            if (std::string(methodName) == ".ctor") {
                m_constructors.emplace_back(new Method(*this, method));
            } else {
                m_methods[methodName] = new Method(*this, method);
            }
        }

        // for (const auto& name : m_methods | std::views::keys) {
        //     std::cout << "Method: " << name << std::endl;
        // }

    }
}
