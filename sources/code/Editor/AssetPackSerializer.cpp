#include <fstream>

#include "AssetPackSerializer.hpp"

#include <Common/Buffer.hpp>
#include <Common/Assets/ArchiveContentFile.hpp>
#include <Common/Assets/ArchiveDirectory.hpp>
#include <Common/Assets/ArchiveDirectoryFile.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "Editor/EditorManager.hpp"

namespace Grindstone::Assets::AssetPackSerializer {
	class AssetPackageSerializer {
	public:
		AssetPackageSerializer(const std::filesystem::path& outputDir, const std::string& archiveName);
		void AddEntry(Editor::AssetRegistry::Entry& entry);
		void WriteBuffer(size_t bufferIndex) const;
		void WriteDirectory();
		uint16_t FindSuitableBufferIndex(uint64_t size);
		size_t GetBufferCount() const;
	protected:
		ArchiveDirectory archiveDirectory;
		std::vector<ResizableBuffer> buffers;
		const uint64_t archiveSize = 1024ul * 1024ul * 200ul; // 200mb

		std::filesystem::path outputDirectory;
		ResizableBuffer resizableStringBuffer{10485760}; // 10mb
		std::string archiveName;
	};

	void SerializeAllAssets(
		const std::filesystem::path& targetPath,
		Editor::BuildProcessStats* buildProgress,
		const float minProgress,
		const float deltaProgress
	) {
		Editor::Manager& editorManager = Editor::Manager::GetInstance();
		const Editor::AssetRegistry& assetRegistry = editorManager.GetAssetRegistry();

		AssetPackageSerializer serializer(targetPath, "TestArchive");

		buildProgress->progress = minProgress;

		{
			std::scoped_lock scopedLock(buildProgress->stringMutex);
			buildProgress->detailText = "Preparing asset map";
		}

		constexpr uint16_t assetTypeCount = static_cast<uint16_t>(AssetType::Count);
		const float segmentProgressFraction = deltaProgress / (3.0f * assetTypeCount);
		for (uint16_t i = 0; i < assetTypeCount; ++i) {
			const AssetType assetType = static_cast<AssetType>(i);
			std::vector<Editor::AssetRegistry::Entry> outEntries;
			assetRegistry.FindAllFilesOfType(assetType, outEntries);

			const size_t assetOfTypeCount = outEntries.size();
			const float assetProgressFraction = segmentProgressFraction * (1.0f / static_cast<float>(assetOfTypeCount));

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

		const size_t bufferCount = serializer.GetBufferCount();
		for (size_t i = 0; i < bufferCount; ++i) {
			buildProgress->progress = minProgress + deltaProgress * (static_cast<float>(i) /static_cast<float>(bufferCount)) + 1.0f / 3.0f;
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

	AssetPackageSerializer::AssetPackageSerializer(const std::filesystem::path& outputDir, const std::string& archiveName) :
		outputDirectory(outputDir), archiveName(archiveName) {}

	void AssetPackageSerializer::AddEntry(Editor::AssetRegistry::Entry& entry) {
		const size_t assetType = static_cast<size_t>(entry.assetType);
		const uint32_t crc = 0;

		const std::filesystem::path actualFilePath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / entry.uuid.ToString();
		if (!std::filesystem::exists(actualFilePath)) {
			return;
		}

		Grindstone::Buffer loadedFile = Utils::LoadFile(actualFilePath.string().c_str());
		const uint64_t size = loadedFile.GetCapacity();
		const uint16_t archiveIndex = FindSuitableBufferIndex(size);
		ResizableBuffer& buffer = buffers[archiveIndex];

		const Byte* dstPtr = static_cast<Byte*>(buffer.AddToBuffer(loadedFile.Get(), size));
		const uint64_t offset = dstPtr - buffer.Get();

		const std::string pathAsStr = entry.address;
		const char* copiedStringPtr = static_cast<const char*>(resizableStringBuffer.AddToBuffer(pathAsStr.c_str(), pathAsStr.size() + 1));
		std::map<Uuid, ArchiveDirectory::AssetInfo>& assetTypeMap = archiveDirectory.assetTypeIndices[assetType].assetsByUuid;
		assetTypeMap[entry.uuid] = {
			copiedStringPtr,
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
		buffers.emplace_back(newArchiveSize);

		return static_cast<uint16_t>(buffers.size() - 1);
	}

	void AssetPackageSerializer::WriteBuffer(const size_t bufferIndex) const {
		const ResizableBuffer& buffer = buffers[bufferIndex];
		const std::string outputFilename = archiveName + "_" + std::to_string(bufferIndex) + ".garc";
		const std::filesystem::path outputPath = outputDirectory / outputFilename;

		std::ofstream output(outputPath, std::ios::binary);

		if (!output.is_open()) {
			throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
		}

		output.write(reinterpret_cast<const char*>(buffer.Get()), static_cast<std::streamsize>(buffer.GetUsedSize()));
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
			size_t count = archiveDirectory.assetTypeIndices[i].assetsByUuid.size();
			outputFile.assetInfoIndex[i].offset = currentAssetOffset;
			outputFile.assetInfoIndex[i].count = static_cast<uint16_t>(count);

			currentAssetOffset += count * sizeof(outputFile.assets[0]);
			totalAssetCount += archiveDirectory.assetTypeIndices[i].assetsByUuid.size();
		}

		outputFile.assets.reserve(totalAssetCount);

		uint64_t assetIndexSize = sizeof(ArchiveDirectoryFile::AssetInfo) * totalAssetCount;
		uint64_t archiveIndexSize = sizeof(ArchiveDirectoryFile::ArchiveInfo) * buffers.size();

		uint64_t archivesBaseOffset = assetBaseOffset + assetIndexSize;
		uint64_t stringBaseOffset = archivesBaseOffset + archiveIndexSize;

		// Prepare actual assets
		for (uint16_t assetTypeIndex = 0; assetTypeIndex < assetTypeCount; ++assetTypeIndex) {
			for (const auto& [uuid, asset] : archiveDirectory.assetTypeIndices[assetTypeIndex].assetsByUuid) {
				uint64_t addressOffset = static_cast<uint64_t>(asset.address.data() - reinterpret_cast<const char*>(resizableStringBuffer.Get()));
				uint16_t addressSize = static_cast<uint16_t>(asset.address.size());

				outputFile.assets.emplace_back(
					ArchiveDirectoryFile::AssetInfo{
						uuid,
						addressOffset,
						addressSize,
						asset.crc,
						asset.archiveIndex,
						asset.offset,
						asset.size
					}
				);
			}
		}

		// Prepare Archives
		for (const auto& [crc] : archiveDirectory.archives) {
			outputFile.archives.emplace_back(ArchiveDirectoryFile::ArchiveInfo{ crc });
		}

		std::ofstream output(outputPath, std::ios::binary);

		if (!output.is_open()) {
			throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
		}

		// Write Header
		{
			outputFile.header.headerSize			= sizeof(ArchiveDirectoryFile::Header);
			outputFile.header.version				= ArchiveDirectoryFile::CURRENT_VERSION;
			outputFile.header.buildCode				= 0;
			outputFile.header.assetTypeCount		= static_cast<uint32_t>(outputFile.assetInfoIndex.size());
			outputFile.header.assetTypeIndexSize	= static_cast<uint32_t>(assetTypeIndexSize);
			outputFile.header.assetInfoIndexSize = static_cast<uint32_t>(assetIndexSize);
			outputFile.header.archiveIndexSize = static_cast<uint32_t>(archiveIndexSize);
			outputFile.header.stringsSize = static_cast<uint32_t>(resizableStringBuffer.GetUsedSize() + 1);
			output.write(reinterpret_cast<const char*>(&outputFile.header), static_cast<std::streamsize>(headerSize));
		}

		// Write Asset Type Index
		output.write(reinterpret_cast<const char*>(outputFile.assetInfoIndex.data()), static_cast<std::streamsize>(assetTypeIndexSize));

		// Write Asset Maps
		output.write(reinterpret_cast<const char*>(outputFile.assets.data()), static_cast<std::streamsize>(assetIndexSize));

		// Write Archive CRCs
		output.write(reinterpret_cast<const char*>(&outputFile.archives), static_cast<std::streamsize>(archiveIndexSize));

		// Write Strings
		output.write(reinterpret_cast<const char*>(resizableStringBuffer.Get()), outputFile.header.stringsSize);

		output.close();
	}

	size_t AssetPackageSerializer::GetBufferCount() const {
		return buffers.size();
	}
}
