#pragma once

#include <deque>
#include <filesystem>
#include <mutex>

namespace Grindstone::Importers {
	class ImportScheduler {
	public:
		bool QueueImport(const std::filesystem::path& path);
		bool Import(const std::filesystem::path& path);
		void ProcessImport();
	private:
		std::deque<std::filesystem::path> pendingImports;
		std::mutex queueMutex;
	};
}
