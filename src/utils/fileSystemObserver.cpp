//
// Created by Eduard Andrei Radu on 06.01.2026.
//

#include "fileSystemObserver.h"

#include "context.h"

namespace Coral::Utils {
	FileSystemObserver::FileSystemObserver() {
		static bool firstTime = true;
		if (!firstTime) {
			throw std::runtime_error("FileSystemObserver already created!");
		}
		firstTime = false;
		Context::m_fileSystemObserver = this;

	}

	void FileSystemObserver::TrackFile(const std::filesystem::path& path) {
		const auto timestamp = std::filesystem::last_write_time(path);
		m_trackedFiles.emplace(path, FileData{ timestamp, false });
	}

	void FileSystemObserver::UntrackFile(const std::filesystem::path& path) {
		m_trackedFiles.erase(path);
	}

	bool FileSystemObserver::HasFileChanged(const std::filesystem::path& path) const {
		return m_trackedFiles.contains(path) && m_trackedFiles.find(path)->second.hasChanged;
	}

	void FileSystemObserver::Update() {
		for (auto& [path, data] : m_trackedFiles) {
			if (const bool exists = std::filesystem::exists(path);
				!exists)
			{
				data.hasChanged = true;
				continue;
			}
			if (const auto timestamp = std::filesystem::last_write_time(path);
				timestamp != data.lastWriteTime)
			{
				data.lastWriteTime = timestamp;
				data.hasChanged = true;
			} else {
				data.hasChanged = false;
			}
		}
	}
}
