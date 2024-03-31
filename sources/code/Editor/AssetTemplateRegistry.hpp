#pragma once

#include <string_view>
#include <filesystem>
#include <vector>

#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone::Editor {
	class AssetTemplateRegistry {
	public:
		void RegisterTemplate(
			AssetType assetType,
			std::string_view name,
			std::string_view extension,
			const void* const sourcePtr, size_t sourceSize
		);
		void RemoveTemplate(AssetType assetType);

		struct AssetTemplate {
			AssetType assetType = AssetType::Undefined;
			std::string name;
			std::string extension;
			std::vector<char> content;

			AssetTemplate(
				AssetType assetType,
				std::string_view name,
				std::string_view extension,
				const void* const sourcePtr,
				size_t sourceSize
			);
		};

		using TemplateList = std::vector<AssetTemplate>;

		virtual TemplateList::iterator begin();
		virtual TemplateList::const_iterator begin() const;
		virtual TemplateList::iterator end();
		virtual TemplateList::const_iterator end() const;

	protected:
		TemplateList assetTemplates;
	};
}
