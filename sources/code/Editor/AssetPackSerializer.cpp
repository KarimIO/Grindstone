#include <fstream>

#include "AssetPackSerializer.hpp"

#include <Common/Assets/ArchiveContentFile.hpp>
#include <Common/Assets/ArchiveDirectory.hpp>
#include <Common/Assets/ArchiveDirectoryFile.hpp>
#include <Common/Buffer.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "Editor/EditorManager.hpp"

namespace Grindstone::Assets::AssetPackSerializer {
	class AssetPackageSerializer {
	public:
		AssetPackageSerializer(std::filesystem::path outputDir, std::string archiveName);
		void AddEntry(Editor::AssetRegistry::Entry& entry);
		void WriteBuffer(size_t i);
		void WriteDirectory();
		uint16_t FindSuitableBufferIndex(uint64_t size);
		size_t GetBufferCount() const;
	protected:
		ArchiveDirectory archiveDirectory;
		std::vector<ResizableBuffer> buffers;
		const uint64_t archiveSize = 1024 * 1024 * 200; // 200mb

		std::filesystem::path outputDirectory;
		std::string archiveName;
	};

	void SerializeAllAssets(std::filesystem::path targetPath, Editor::BuildProcessStats* buildProgress, float minProgress, float deltaProgress) {
		Editor::Manager& editorManager = Editor::Manager::GetInstance();
		Editor::AssetRegistry& assetRegistry = editorManager.GetAssetRegistry();

		AssetPackageSerializer serializer(targetPath, "TestArchive");

		buildProgress->progress = minProgress;

		{
			std::scoped_lock scopedLock(buildProgress->stringMutex);
			buildProgress->detailText = "Preparing asset map";
		}
		uint16_t assetTypeCount = static_cast<uint16_t>(AssetType::Count);
		float segmentProgressFraction = deltaProgress / (assetTypeCount * 3.0f);
		for (uint16_t i = 0; i < assetTypeCount; ++i) {
			AssetType assetType = static_cast<AssetType>(i);
			std::vector<Editor::AssetRegistry::Entry> outEntries;
			assetRegistry.FindAllFilesOfType(assetType, outEntries);

			size_t assetOfTypeCount = outEntries.size();
			float assetProgressFraction = segmentProgressFraction * (1.0f / assetOfTypeCount);

			for (size_t assetIndex = 0; assetIndex < assetOfTypeCount; ++assetIndex) {
				serializer.AddEntry(outEntries[assetIndex]);
				buildProgress->progress = buildProgress->progress + assetProgressFraction;
			}
		}

		buildProgress->progress = minProgress + deltaProgress / 3.0f;
		{
			std::scoped_lock scopedLock(buildProgress->stringMutex);
			buildProgress->detailText = "Writing archive buffers";
		}

		size_t bufferCount = serializer.GetBufferCount();
		for (size_t i = 0; i < bufferCount; ++i) {
			buildProgress->progress = minProgress + deltaProgress * (static_cast<float>(i) / bufferCount) + (1 / 3.0f);
			serializer.WriteBuffer(i);
		}

		buildProgress->progress = minProgress + (2.0f * deltaProgress) / 3.0f;

		{
			std::scoped_lock scopedLock(buildProgress->stringMutex);
			buildProgress->detailText = "Writing archive directory";
		}
		serializer.WriteDirectory();

		buildProgress->progress = minProgress + deltaProgress;
	}

	AssetPackageSerializer::AssetPackageSerializer(std::filesystem::path outputDir, std::string archiveName) :
		outputDirectory(outputDir), archiveName(archiveName) {}

	void AssetPackageSerializer::AddEntry(Editor::AssetRegistry::Entry& entry) {
		size_t assetType = static_cast<size_t>(entry.assetType);
		uint32_t crc = 0;

		std::filesystem::path actualFilePath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / entry.uuid.ToString();
		if (!std::filesystem::exists(actualFilePath)) {
			return;
		}

		std::vector<char> loadedFile = Utils::LoadFile(actualFilePath.string().c_str());
		uint64_t size = loadedFile.size();
		uint16_t archiveIndex = FindSuitableBufferIndex(size);
		ResizableBuffer& buffer = buffers[archiveIndex];

		Byte* dstPtr = static_cast<Byte*>(buffer.AddToBuffer(loadedFile.data(), size));
		uint64_t offset = dstPtr - buffer.Get();

		std::string& path = entry.path.string();
		std::map<Uuid, ArchiveDirectory::AssetInfo>& assetTypeMap = archiveDirectory.assetTypeIndices[assetType].assets;
		assetTypeMap[entry.uuid] = {
			path,
			crc,
			archiveIndex,
			offset,
			size
		};
	}

	uint16_t AssetPackageSerializer::FindSuitableBufferIndex(uint64_t size) {
		for (size_t i = 0; i < buffers.size(); ++i) {
			if (buffers[i].GetSpaceLeft() >= size) {
				return static_cast<uint16_t>(i);
			}
		}

		// If this asset size is bigger than an entire archive (this probably shouldn't happen), then make the archive just this file.
		uint64_t newArchiveSize = size > archiveSize
			? size
			: archiveSize;

		// If no available buffer has enough space, add a new one
		buffers.emplace_back(ResizableBuffer(newArchiveSize));

		return static_cast<uint16_t>(buffers.size() - 1);
	}

	void AssetPackageSerializer::WriteBuffer(size_t i) {
		ResizableBuffer& buffer = buffers[i];
		std::string outputFilename = archiveName + "_" + std::to_string(i) + ".garc";
		std::filesystem::path outputPath = outputDirectory / outputFilename;

		std::ofstream output(outputPath, std::ios::binary);

		if (!output.is_open()) {
			throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
		}

		output.write(reinterpret_cast<const char*>(buffer.Get()), buffer.GetUsedSize());
	}

	void AssetPackageSerializer::WriteDirectory() {
		std::string outputFilename = archiveName + ".gdir";
		std::filesystem::path outputPath = outputDirectory / outputFilename;

		uint16_t assetTypeCount = static_cast<uint16_t>(AssetType::Count);

		ArchiveDirectoryFile outputFile;
		outputFile.assetInfoIndex.resize(assetTypeCount);
		outputFile.archives.reserve(buffers.size());

		uint64_t headerSize = sizeof(outputFile.header);
		uint64_t assetTypeIndexSize = sizeof(ArchiveDirectoryFile::AssetTypeSectionInfo) * assetTypeCount;
		uint64_t assetBaseOffset = headerSize + assetTypeIndexSize;
		uint64_t currentAssetOffset = assetBaseOffset;
		uint64_t totalAssetCount = 0;
		for (uint16_t i = 0; i < assetTypeCount; ++i) {
			size_t count = archiveDirectory.assetTypeIndices[i].assets.size();
			outputFile.assetInfoIndex[i].offset = currentAssetOffset;
			outputFile.assetInfoIndex[i].count = static_cast<uint16_t>(count);

			currentAssetOffset += count * sizeof(outputFile.assets[0]);
			totalAssetCount += archiveDirectory.assetTypeIndices[i].assets.size();
		}

		outputFile.assets.reserve(totalAssetCount);

		std::string outputString = "";

		uint64_t assetIndexSize = sizeof(outputFile.assets) * totalAssetCount;
		uint64_t archiveIndexSize = sizeof(outputFile.archives[0]) * buffers.size();

		uint64_t archivesBaseOffset = assetBaseOffset + sizeof(outputFile.assets) * totalAssetCount;
		uint64_t stringBaseOffset = assetBaseOffset + sizeof(outputFile.archives[0]) * buffers.size();

		uint64_t currentAssetIndex = 0;
		// Prepare actual assets
		for (uint16_t i = 0; i < assetTypeCount; ++i) {
			uint64_t offset = outputFile.assetInfoIndex[i].offset;
			for (auto& [uuid, asset] : archiveDirectory.assetTypeIndices[i].assets) {
				uint64_t filenameOffset = stringBaseOffset + outputString.size();
				outputString += asset.filename;

				uint16_t filenameSize = static_cast<uint16_t>(asset.filename.size());

				outputFile.assets.emplace_back(
					ArchiveDirectoryFile::AssetInfo{
						uuid,
						filenameOffset,
						filenameSize,
						asset.crc,
						asset.archiveIndex,
						asset.offset,
						asset.size
					}
				);
			}
		}

		// Prepare Archives
		for (auto& archive : archiveDirectory.archives) {
			outputFile.archives.emplace_back(ArchiveDirectoryFile::ArchiveInfo{ archive.crc });
		}

		std::ofstream output(outputPath);

		// Write Header
		{
			outputFile.header.assetTypeIndexSize	= static_cast<uint32_t>(assetTypeIndexSize);
			outputFile.header.assetInfoIndexSize	= static_cast<uint32_t>(assetIndexSize);
			outputFile.header.archiveIndexSize		= static_cast<uint32_t>(archiveIndexSize);
			output.write(reinterpret_cast<const char*>(&outputFile.header), headerSize);
		}

		// Write Asset Type Index
		output.write(reinterpret_cast<const char*>(outputFile.assetInfoIndex.data()), assetTypeIndexSize);
		
		// Write Asset Maps
		output.write(reinterpret_cast<const char*>(outputFile.assets.data()), assetIndexSize);
		
		// Write Archive CRCs
		output.write(reinterpret_cast<const char*>(&outputFile.archives), archiveIndexSize);
		
		// Write Strings
		output.write(outputString.c_str(), outputString.size());
		
		output.close();
	}

	size_t AssetPackageSerializer::GetBufferCount() const {
		return buffers.size();
	}
}
