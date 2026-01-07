//
// Created by Eduard Andrei Radu on 06.01.2026.
//

#pragma once

#include <unordered_map>
#include <filesystem>

namespace Coral::Utils {
	struct FileData {
		std::filesystem::file_time_type lastWriteTime;
		bool hasChanged = false;
	};

	class FileSystemObserver {
	public:
		FileSystemObserver();

		void TrackFile(const std::filesystem::path& path);
		void UntrackFile(const std::filesystem::path& path);

		bool HasFileChanged(const std::filesystem::path& path) const;
		void Update();

	private:
		std::unordered_multimap<std::filesystem::path, FileData> m_trackedFiles {};
	};
}
