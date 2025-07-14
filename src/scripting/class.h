//
// Created by radue on 4/22/2025.
//
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include "domain.h"

namespace Coral::Scripting {
    class RemoteVoid;
	class Class;

	template<class T>
    class Remote;

    template<typename T>
    concept RemoteClass = std::is_base_of_v<Remote<T>, T>;

    class ClassField {
        friend class Class;
    public:
        ~ClassField() = default;

        [[nodiscard]] MonoType* Type() const {
            return mono_field_get_type(m_field);
        }

        MonoClassField* operator*() const { return m_field; }

    private:
        ClassField(MonoClass* klass, const std::string& name);

        MonoType* m_type;
        MonoClassField* m_field;
    };

    class Object;

    class Method {
        friend class Class;
    public:
        ~Method() = default;

        [[nodiscard]] MonoType* ReturnType() const {
            return mono_signature_get_return_type(mono_method_signature(m_method));
        }

        [[nodiscard]] const std::vector<MonoType*>& Parameters() const {
            return m_parameters;
        }

        template<RemoteClass T, RemoteClass... Args>
        std::unique_ptr<T> Invoke(MonoObject* instance, Args*... args) const{
            if (sizeof...(args) != m_parameters.size()) {
                throw std::runtime_error("Invalid number of arguments");
            }

        	if constexpr (sizeof...(args) == 0) {
        		MonoObject* exception = nullptr;
        		MonoObject* result = mono_runtime_invoke(m_method, instance, nullptr, &exception);
        		if (exception != nullptr) {
        			mono_print_unhandled_exception(exception);
        			throw std::runtime_error("Failed to invoke method");
        		}
        		if (result == nullptr) {
        			return nullptr;
        		}
        		return std::unique_ptr<T>(Remote<T>::LocalInstance(result));
        	} else {
        		MonoArray* argsArray = mono_array_new(*Domain::Get(), mono_get_object_class(), sizeof...(args));
        		int i = 0;
        		for (const auto& arg : {args...}) {
        			mono_array_set(argsArray, MonoObject*, i++, **arg->RemoteInstance());
        		}

        		MonoObject* exception = nullptr;
        		MonoObject* result = mono_runtime_invoke_array(m_method, instance, argsArray, &exception);
        		if (exception != nullptr) {
        			mono_print_unhandled_exception(exception);
        			throw std::runtime_error("Failed to invoke method");
        		}
        		if (result == nullptr) {
        			return nullptr;
        		}
        		return std::unique_ptr<T>(Remote<T>::LocalInstance(result));
        	}
        }

        [[nodiscard]] const Class& Class() const {
            return m_class;
        }

    private:
        const Scripting::Class& m_class;
        std::string m_name;
        Method(const Scripting::Class& klass, MonoMethod* method);

        MonoMethod* m_method;
        std::vector<MonoType*> m_parameters;
    };

    class Class {
        friend class Assembly;
    public:
        ~Class() = default;

    	MonoClass* operator*() const { return m_class; }

    	std::string Name() const {
    		return mono_class_get_name(m_class);
    	}

        [[nodiscard]] const Method& GetMethod(const std::string& name) const {
            const auto method = m_methods.find(name);
            if (method == m_methods.end()) {
                throw std::runtime_error("Method not found: " + name);
            }
            return *method->second;
        }

        [[nodiscard]] ClassField* GetField(const std::string& name) const {
            const auto field = m_fields.find(name);
            if (field == m_fields.end()) {
                throw std::runtime_error("Field not found: " + name);
            }
            return field->second;
        }

        [[nodiscard]] MonoType* GetType() const {
            return mono_class_get_type(m_class);
        }

        [[nodiscard]] Object* CreateInstance() const;

        template<RemoteClass T = RemoteVoid, RemoteClass... Args>
        std::unique_ptr<T> Call(const std::string& methodName, Args*... args) const {
            return m_methods.at(methodName)->Invoke<T>(nullptr, std::forward<Args*>(args)...);
        }

        void PrintInfo() const;

    private:
        Class(MonoImage* image, const std::string& namespac, const std::string& name);

        std::vector<Method*> m_constructors;
        std::unordered_map<std::string, ClassField*> m_fields;
        std::unordered_map<std::string, Method*> m_methods;
        // std::unordered_map<std::string, Property*> m_properties;

        MonoClass* m_class;
    };
}
