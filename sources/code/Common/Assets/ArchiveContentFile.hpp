#pragma once

#include <string>
#include <map>

#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::Assets {
	struct ArchiveContentFile {
		const static uint32_t CURRENT_VERSION = 1;

		struct Header {
			const char signature[4] = { 'G', 'A', 'R', 'C' };
			uint32_t version;
			uint32_t buildCode;
			uint32_t headerSize;
			uint32_t contentSize;
			uint32_t crc;

			Header() {
				version = CURRENT_VERSION;
			}
		};

		Header header;
	};
}
