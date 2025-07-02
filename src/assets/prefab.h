//
// Created by radue on 6/25/2025.
//

#pragma once

#include <boost/uuid/string_generator.hpp>
#include <nlohmann/json.hpp>

namespace Coral::Asset {
	class Prefab {
	public:
		explicit Prefab(std::string name, nlohmann::json metadata);
		void Load() const;

		[[nodiscard]] const std::string& Name() const { return m_name; }

	private:
		std::string m_name;
		nlohmann::json m_metadata;
        inline static boost::uuids::string_generator _stringToUuid;
	};

}

