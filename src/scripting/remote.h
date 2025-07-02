//
// Created by radue on 5/6/2025.
//

#pragma once

#include <mono/metadata/object-forward.h>

#include <stdexcept>
#include <unordered_map>

#include "object.h"

namespace Coral {
	class Engine;
}
namespace Coral::Scripting {
	class Class;
	class Object;

	template<typename T>
	class Remote {
		friend void LinkClassesWithRemote();
	public:
		~Remote() {
			if (g_remoteInstances.contains(static_cast<T*>(this))) {
				const Object* obj = g_remoteInstances.at(static_cast<T*>(this));
				 std::cout << "Destroying remote instance: " << obj->GetClass().Name() << std::endl;
				g_localInstances.erase(**obj);
				g_remoteInstances.erase(static_cast<T*>(this));
				delete obj;
			}
		}

		[[nodiscard]] static T* LocalInstance(MonoObject* obj) {
			if (g_localInstances.contains(obj)) {
				return g_localInstances.at(obj);
			}
			throw std::runtime_error("Local instance not found");
		}

		[[nodiscard]] Object* RemoteInstance() {
			T* self = static_cast<T*>(this);
			if (g_remoteInstances.contains(self)) {
				return g_remoteInstances.at(self);
			}
			auto obj = g_remoteInstances.emplace(self, new Object(*s_class)).first->second;
			g_localInstances.emplace(**obj, self);
			return obj;
		}

		static Class* RemoteClass() {
			return s_class;
		}

		static int LocalInstanceCount() {
			return static_cast<int>(g_localInstances.size());
		}
		static int RemoteInstanceCount() {
			return static_cast<int>(g_remoteInstances.size());
		}

		void AddRemoteInstance(MonoObject* obj) {
			if (g_localInstances.contains(obj)) {
				throw std::runtime_error("Local instance already exists");
			}
			g_localInstances[obj] = static_cast<T*>(this);
			g_remoteInstances[static_cast<T*>(this)] = new Object(*s_class, obj);
		}

	private:
		inline static Class* s_class = nullptr;

		inline static std::unordered_map<MonoObject*, T*> g_localInstances;
		inline static std::unordered_map<T*, Object*> g_remoteInstances;
	};

	class RemoteVoid : public Remote<RemoteVoid> {};
}