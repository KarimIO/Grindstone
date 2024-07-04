#pragma once

#include <filesystem>
#include <vector>

namespace Grindstone::Editor::Importers {
	enum class OutputFormat {
		Undefined = 0,
		BC1,
		BC3,
		BC4,
		BC6H
	};

	struct DdsFileWriterOptions {
		uint32_t texWidth = 0;
		uint32_t texHeight = 0;
		uint32_t texDepth = 0;
		uint32_t mipmapCount = 0;
		bool isCubemap = false;
		OutputFormat outputFormat = OutputFormat::Undefined;
	};

	void WriteDdsFile(const std::filesystem::path& outputPath, const std::vector<uint8_t>& fileContents, const DdsFileWriterOptions& options);
}
