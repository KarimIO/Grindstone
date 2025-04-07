#include <fmt/format.h>
#include <set>

#include "StateResolver.hpp"
#include "ParseTree.hpp"

constexpr size_t maxInheritance = 32;
const Grindstone::GraphicsAPI::ColorMask defaultColorMask = Grindstone::GraphicsAPI::ColorMask::RGBA;
const Grindstone::GraphicsAPI::BlendOperation defaultBlendOperation = Grindstone::GraphicsAPI::BlendOperation::None;
const Grindstone::GraphicsAPI::BlendFactor defaultBlendFactor = Grindstone::GraphicsAPI::BlendFactor::Zero;

template<typename T>
inline static void ApplyPropertyDefault(std::optional<T>& dst, T src) {
	if (!dst.has_value()) {
		dst = src;
	}
}

template<typename T>
inline static void ApplyPropertyImpl(const std::optional<T>& src, std::optional<T>& dst) {
	if (!dst.has_value()) {
		dst = src;
	}
}

template<typename T>
inline static void ApplyPropertyWithDefault(const std::optional<T>& src, std::optional<T>& dst, T defaultValue) {
	if (!dst.has_value()) {
		if (src.has_value()) {
			dst = src;
		}
		else {
			dst = defaultValue;
		}
	}
}

#define ApplyProperty(memberName) ApplyPropertyImpl(src.memberName, dst.memberName)

static void ApplyRenderState(const ParseTree::RenderState& src, ParseTree::RenderState& dst) {
	ApplyProperty(primitiveType);
	ApplyProperty(polygonFillMode);
	ApplyProperty(cullMode);

	ApplyProperty(depthCompareOp);
	ApplyProperty(isStencilEnabled);
	ApplyProperty(isDepthTestEnabled);
	ApplyProperty(isDepthWriteEnabled);
	ApplyProperty(isDepthBiasEnabled);
	ApplyProperty(isDepthClampEnabled);

	ApplyProperty(depthBiasConstantFactor);
	ApplyProperty(depthBiasSlopeFactor);
	ApplyProperty(depthBiasClamp);

	/*
	if (dst.attachmentData.size() == 0) {
		dst.attachmentData.resize(src.attachmentData.size());
	}

	size_t minAttachmentCount = dst.attachmentData.size() > src.attachmentData.size()
		? src.attachmentData.size()
		: dst.attachmentData.size();

	for (size_t i = 0; i < minAttachmentCount; ++i) {
		ParseTree::RenderState::AttachmentData& srcAttach = src.attachmentData[i];
		ResolvedStateTree::RenderState::AttachmentData& dstAttach = dst.attachmentData[i];

		ApplyPropertyWithDefault(srcAttach.colorMask, dstAttach.colorMask, defaultColorMask);

		ApplyPropertyWithDefault(srcAttach.blendColorOperation, dstAttach.blendColorOperation, defaultBlendOperation);
		ApplyPropertyWithDefault(srcAttach.blendColorFactorSrc, dstAttach.blendColorFactorSrc, defaultBlendFactor);
		ApplyPropertyWithDefault(srcAttach.blendColorFactorDst, dstAttach.blendColorFactorDst, defaultBlendFactor);

		ApplyPropertyWithDefault(srcAttach.blendAlphaOperation, dstAttach.blendAlphaOperation, defaultBlendOperation);
		ApplyPropertyWithDefault(srcAttach.blendAlphaFactorSrc, dstAttach.blendAlphaFactorSrc, defaultBlendFactor);
		ApplyPropertyWithDefault(srcAttach.blendAlphaFactorDst, dstAttach.blendAlphaFactorDst, defaultBlendFactor);
	}

	for (size_t i = minAttachmentCount; i < dst.attachmentData.size(); ++i) {
		ParseTree::RenderState::RenderState::AttachmentData& dstAttach = src.attachmentData[i];

		ApplyPropertyDefault(dstAttach.colorMask, defaultColorMask);

		ApplyPropertyDefault(dstAttach.blendColorOperation, defaultBlendOperation);
		ApplyPropertyDefault(dstAttach.blendColorFactorSrc, defaultBlendFactor);
		ApplyPropertyDefault(dstAttach.blendColorFactorDst, defaultBlendFactor);

		ApplyPropertyDefault(dstAttach.blendAlphaOperation, defaultBlendOperation);
		ApplyPropertyDefault(dstAttach.blendAlphaFactorSrc, defaultBlendFactor);
		ApplyPropertyDefault(dstAttach.blendAlphaFactorDst, defaultBlendFactor);
	}
	*/
}

static void ApplyRenderStateDefaults(ParseTree::RenderState& target) {
	ApplyPropertyDefault(target.primitiveType, Grindstone::GraphicsAPI::GeometryType::Triangles);
	ApplyPropertyDefault(target.polygonFillMode, Grindstone::GraphicsAPI::PolygonFillMode::Fill);
	ApplyPropertyDefault(target.cullMode, Grindstone::GraphicsAPI::CullMode::Back);

	ApplyPropertyDefault(target.depthCompareOp, Grindstone::GraphicsAPI::CompareOperation::GreaterOrEqual);
	ApplyPropertyDefault(target.isStencilEnabled, false);
	ApplyPropertyDefault(target.isDepthTestEnabled, true);
	ApplyPropertyDefault(target.isDepthWriteEnabled, true);
	ApplyPropertyDefault(target.isDepthBiasEnabled, false);
	ApplyPropertyDefault(target.isDepthClampEnabled, false);

	ApplyPropertyDefault(target.depthBiasConstantFactor, 0.0f);
	ApplyPropertyDefault(target.depthBiasSlopeFactor, 0.0f);
	ApplyPropertyDefault(target.depthBiasClamp, 0.0f);

	for (ParseTree::RenderState::AttachmentData& attachment : target.attachmentData) {
		ApplyPropertyDefault(attachment.colorMask, defaultColorMask);

		ApplyPropertyDefault(attachment.blendColorOperation, defaultBlendOperation);
		ApplyPropertyDefault(attachment.blendColorFactorSrc, defaultBlendFactor);
		ApplyPropertyDefault(attachment.blendColorFactorDst, defaultBlendFactor);

		ApplyPropertyDefault(attachment.blendAlphaOperation, defaultBlendOperation);
		ApplyPropertyDefault(attachment.blendAlphaFactorSrc, defaultBlendFactor);
		ApplyPropertyDefault(attachment.blendAlphaFactorDst, defaultBlendFactor);
	}

}

struct ResolveContext {
	LogCallback logCallback;
	const ParseTree& parseTree;
	ResolvedStateTree& resolvedStateTree;
	bool shouldCopyCode = true;

	ResolveContext(
		LogCallback logCallback,
		const ParseTree& parseTree,
		ResolvedStateTree& resolvedStateTree
	) : logCallback(logCallback), parseTree(parseTree), resolvedStateTree(resolvedStateTree) {}
};

static bool HasBrokenDependency(const ResolveContext& context, const std::string& pipelineSetName, const ParseTree::PipelineSet& pipelineSet) {
	std::set<std::string> processedSets;
	const ParseTree::PipelineSet* currentPipelineSet = &pipelineSet;
	bool shouldAppendParentCode = true;

	// This limit is arbitary, to ensure we don't loop forever.
	for (size_t i = 0; i < maxInheritance; ++i) {
		processedSets.insert(pipelineSetName);

		if (currentPipelineSet->parentData.parent.empty()) {
			return false;
		}
		else {
			const std::string& parentName = currentPipelineSet->parentData.parent;

			if (processedSets.find(parentName) != processedSets.end()) {
				std::string msg = fmt::format("Cyclical dependency found for {} while trying to resolve pipelineSet {}.", parentName, pipelineSetName);
				context.logCallback(LogLevel::Error, LogSource::Resolver, msg, pipelineSet.sourceFilepath, UNDEFINED_COLUMN, UNDEFINED_LINE);
				return true;
			}

			auto pipelineIterator = context.parseTree.pipelineSets.find(parentName);
			if (pipelineIterator == context.parseTree.pipelineSets.end()) {
				std::string msg = fmt::format("Invalid parent found for {} while trying to resolve pipelineSet {}.", parentName, pipelineSetName);
				context.logCallback(LogLevel::Error, LogSource::Resolver, msg, currentPipelineSet->sourceFilepath, UNDEFINED_COLUMN, UNDEFINED_LINE);
				return true;
			}

			currentPipelineSet = &pipelineIterator->second;
		}
	}

	{
		std::string msg = fmt::format("Too many dependencies for pipelineSet {}. The dependency chain is over {} deep.", pipelineSetName, maxInheritance);
		context.logCallback(LogLevel::Error, LogSource::Resolver, msg, pipelineSet.sourceFilepath, UNDEFINED_COLUMN, UNDEFINED_LINE);
	}

	return true;
}

static void ResolvePass(ResolveContext& context, const ParseTree::Pass& srcPass, ResolvedStateTree::Pass& dstPass) {
	for (const std::string& requiredBlock : srcPass.shaderBlock.requiredShaderBlocks) {
		dstPass.requiredShaderBlocks.insert(requiredBlock);
	}

	if (context.shouldCopyCode) {
		dstPass.code += srcPass.shaderBlock.code;
	}
	
	for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
		if (dstPass.stageEntryPoints[stageIndex].empty() && !srcPass.shaderBlock.stageEntryPoints[stageIndex].empty()) {
			dstPass.stageEntryPoints[stageIndex] = srcPass.shaderBlock.stageEntryPoints[stageIndex];
		}
	}

	ApplyRenderState(srcPass.renderState, dstPass.renderState);
}

static void ResolveConfiguration(ResolveContext& context, const ParseTree::Configuration& srcConfiguration, ResolvedStateTree::Configuration& dstConfiguration) {
	for (auto& passIterator : srcConfiguration.passes) {
		const std::string& passName = passIterator.first;
		const ParseTree::Pass& srcPass = passIterator.second;
		ResolvedStateTree::Pass& dstPass = dstConfiguration.passes[passName];

		ResolvePass(context, srcPass, dstPass);
	}
}

static void ResolvePipelineSetIteration(ResolveContext& context, const std::string& pipelineSetName, const ParseTree::PipelineSet& pipelineSet, ResolvedStateTree::PipelineSet& resolvedPipelineSet) {
	for (auto& configIterator : pipelineSet.configurations) {
		const std::string& configurationName = configIterator.first;
		const ParseTree::Configuration& srcConfiguration = configIterator.second;
		ResolvedStateTree::Configuration& dstConfiguration = resolvedPipelineSet.configurations[configurationName];

		ResolveConfiguration(context, srcConfiguration, dstConfiguration);
	}
}

static void CollapseShaderBlock(ResolveContext& context, std::string& code, std::set<std::string_view>& processedBlocks, const ParseTree::ShaderBlock& shaderBlock) {
	for (const std::string& requiredShaderBlock : shaderBlock.requiredShaderBlocks) {
		if (processedBlocks.find(requiredShaderBlock) == processedBlocks.end()) {
			continue;
		}

		auto blockIterator = context.parseTree.genericShaderBlocks.find(requiredShaderBlock);
		if (blockIterator == context.parseTree.genericShaderBlocks.end()) {
			std::string msg = fmt::format("Could not find shader block '{}'", requiredShaderBlock);
			context.logCallback(LogLevel::Error, LogSource::Resolver, msg, shaderBlock.sourceFilepath, UNDEFINED_COLUMN, UNDEFINED_LINE);
		}
		else {
			processedBlocks.insert(requiredShaderBlock);
			CollapseShaderBlock(context, code, processedBlocks, blockIterator->second);
		}
	}

	code += shaderBlock.code;
}

static void CollapsePasses(ResolveContext& context, ResolvedStateTree::PipelineSet& resolvedPipelineSet) {
	for (auto& configIterator : resolvedPipelineSet.configurations) {
		for (auto& passIterator : configIterator.second.passes) {
			for (auto& passIterator : configIterator.second.passes) {
				ResolvedStateTree::Pass& pass = passIterator.second;
				std::string code;
				std::set<std::string_view> processedBlocks;
				for (const std::string_view requiredShaderBlock : pass.requiredShaderBlocks) {
					if (processedBlocks.find(requiredShaderBlock) == processedBlocks.end()) {
						continue;
					}

					auto shaderBlockIterator = context.parseTree.genericShaderBlocks.find(std::string(requiredShaderBlock));
					if (shaderBlockIterator == context.parseTree.genericShaderBlocks.end()) {
						std::string errorMsg = fmt::format("Found a missing shader block '{}'.", requiredShaderBlock);
						context.logCallback(LogLevel::Error, LogSource::Resolver, errorMsg, resolvedPipelineSet.sourceFilepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
					}
					else {
						const ParseTree::ShaderBlock& shaderBlock = shaderBlockIterator->second;
						processedBlocks.insert(requiredShaderBlock);
						CollapseShaderBlock(context, code, processedBlocks, shaderBlock);
					}
				}

				code += pass.code;
				pass.code = code;

				ApplyRenderStateDefaults(pass.renderState);
			}
		}
	}
}

static void ResolvePipelineSet(ResolveContext& context, const std::string& pipelineSetName, const ParseTree::PipelineSet& pipelineSet, ResolvedStateTree::PipelineSet& resolvedPipelineSet) {
	if (HasBrokenDependency(context, pipelineSetName, pipelineSet)) {
		return;
	}

	resolvedPipelineSet.name = pipelineSetName;

	context.shouldCopyCode = true;
	const ParseTree::PipelineSet* currentPipelineSet = &pipelineSet;
	while (true) {
		ResolvePipelineSetIteration(context, pipelineSetName, *currentPipelineSet, resolvedPipelineSet);
		
		if (currentPipelineSet->parentData.parent.empty()) {
			break;
		}
		else {
			if (currentPipelineSet->parentData.parentType == ParseTree::ParentType::Inherit) {
				context.shouldCopyCode = false;
			}
		
			const std::string& parent = currentPipelineSet->parentData.parent;
			currentPipelineSet = &context.parseTree.pipelineSets.find(parent)->second;
		}
	}

	CollapsePasses(context, resolvedPipelineSet);
}

bool ResolveStateTree(LogCallback logCallback, const ParseTree& parseTree, ResolvedStateTree& resolvedStateTree) {
	ResolveContext context(logCallback, parseTree, resolvedStateTree);
	
	for (auto& pipelineSetIterator : parseTree.pipelineSets) {
		const ParseTree::PipelineSet& pipelineSet = pipelineSetIterator.second;
		if (pipelineSet.isAbstract) {
			continue;
		}

		const std::string& pipelineSetName = pipelineSetIterator.first;
		ResolvedStateTree::PipelineSet& resolvedPipelineSet = resolvedStateTree.pipelineSets.emplace_back();
		resolvedPipelineSet.sourceFilepath = pipelineSet.sourceFilepath;

		ResolvePipelineSet(context, pipelineSetName, pipelineSet, resolvedPipelineSet);
	}

	return true;
}
