#pragma once

namespace Grindstone::Editor::Importers {
	enum class SourceChannelDataType {
		Undefined = 0,
		Uint8,
		Float
	};

	enum class OutputFormat {
		Undefined = 0,
		BC1,
		BC3,
		BC4,
		BC6H
	};
}
