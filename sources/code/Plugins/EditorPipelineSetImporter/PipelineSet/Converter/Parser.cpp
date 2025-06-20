#include <string>
#include <array>
#include <map>
#include <fmt/format.h>

#include "Parser.hpp"

constexpr size_t MAXIMUM_ATTACHMENT_COUNT = 32;

struct ParseContext {
	TokenList& scannerTokens;
	size_t tokenIterator = 0;
	const std::filesystem::path& filepath;

	ParseTree& parseTree;
	LogCallback Log = nullptr;

	ParseContext(TokenList& scannerTokens, const std::filesystem::path& filepath, LogCallback logFn, ParseTree& parseTree) : scannerTokens(scannerTokens), filepath(filepath), parseTree(parseTree), Log(logFn) {}
};

static std::string PrintToken(TokenData& tokenData) {
	std::string token = tokenStrings[static_cast<size_t>(tokenData.token)];
	switch (tokenData.token) {
	case Token::Boolean:
		return token + "(" + (tokenData.data.boolean ? "true" : "false") + ")";
	case Token::Number:
		return token + "(" + std::to_string(tokenData.data.number) + ")";
	case Token::Identifier:
	case Token::String:
		return token + "(" + std::string(tokenData.data.string) + ")";
	default:
		return token;
	}
}

static void UnexpectedError(ParseContext& context, TokenData& token) {
	context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, fmt::format("Unexpected token: {}", PrintToken(token)), token.path, token.line, token.column);
}

static void UnexpectedError(ParseContext& context) {
	if (context.tokenIterator >= context.scannerTokens.size()) {
		if (!context.scannerTokens.empty()) {
			TokenData token = context.scannerTokens[context.scannerTokens.size() - 1];
			context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, fmt::format("Unexpected end of line after token: ", PrintToken(token)), token.path, token.line, token.column);
		}
		else {
			context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, "Unexpected end of line: ", context.filepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
		}
	}

	TokenData& token = context.scannerTokens[context.tokenIterator];
	UnexpectedError(context, token);
}

static bool ExpectToken(ParseContext& context, Token token, const char* errorMsg = nullptr) {
	if (context.tokenIterator >= context.scannerTokens.size()) {
		if (errorMsg != nullptr) {
			std::string errorMsgExtended;
			errorMsgExtended = std::string(errorMsg) + " - Found end of file.";
			context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, errorMsg, context.filepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
		}
		return false;
	}

	TokenData& tokenData = context.scannerTokens[context.tokenIterator];
	if (tokenData.token != token) {
		if (errorMsg != nullptr) {
			std::string errorMsgExtended;
			errorMsgExtended = std::string(errorMsg) + " - Expected token '" + tokenStrings[static_cast<size_t>(token)] + ", found token " + PrintToken(tokenData) + ".";
			context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, errorMsg, tokenData.path, tokenData.line, tokenData.column);
		}
		return false;
	}

	context.tokenIterator++;
	return true;
}

static bool ExpectColon(ParseContext& context, const char* errorMsg) {
	return ExpectToken(context, Token::Colon, errorMsg);
}

static bool ExpectToken(ParseContext& context, Token token, TokenData::Data& outData, const char* errorMsg = nullptr) {
	if (context.tokenIterator >= context.scannerTokens.size()) {
		return false;
	}

	TokenData& tokenData = context.scannerTokens[context.tokenIterator];
	if (tokenData.token != token) {
		if (errorMsg != nullptr) {
			std::string errorTotalMsg = errorMsg;
			errorTotalMsg += " - found token " + PrintToken(tokenData) + ".";
			context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, errorTotalMsg.c_str(), tokenData.path, tokenData.line, tokenData.column);
		}
		return false;
	}

	outData = tokenData.data;
	context.tokenIterator++;
	return true;
}

static bool ExpectBoolean(ParseContext& context, bool& outData, const char* errorMsg = nullptr) {
	TokenData::Data tempData;
	if (ExpectToken(context, Token::Boolean, tempData, errorMsg)) {
		outData = tempData.boolean;
		return true;
	}

	return false;
}

static bool ExpectTokenWithString(ParseContext& context, Token token, std::string_view& outData, const char* errorMsg = nullptr) {
	TokenData::Data tempData;
	if (ExpectToken(context, token, tempData, errorMsg)) {
		outData = tempData.string;
		return true;
	}

	return false;
}

static bool ExpectParameter(ParseContext& context, ParameterType& parameterType) {
	TokenData::Data outData;
	if (ExpectToken(context, Token::Parameter, outData, "Expected a Parameter")) {
		parameterType = outData.parameterType;
		return true;
	}

	return false;
}

static void ExpectCompareOp(ParseContext& context, std::optional<Grindstone::GraphicsAPI::CompareOperation>& compareOp) {
	TokenData::Data outData;
	if (ExpectToken(context, Token::CompareOperationValue, outData, "Expected a comparison operator")) {
		compareOp = outData.compareOperation;
	}
}

static void ExpectGeometryType(ParseContext& context, std::optional<Grindstone::GraphicsAPI::GeometryType>& geometry) {
	TokenData::Data outData;
	if (ExpectToken(context, Token::GeometryTypeValue, outData, "Expected a geometry type")) {
		geometry = outData.geometryType;
	}
}

static void ExpectFillMode(ParseContext& context, std::optional<Grindstone::GraphicsAPI::PolygonFillMode>& fill) {
	TokenData::Data outData;
	if (ExpectToken(context, Token::FillModeValue, outData, "Expected a fill mode")) {
		fill = outData.polygonFillMode;
	}
}

static bool ExpectBlendOp(ParseContext& context, std::optional<Grindstone::GraphicsAPI::BlendOperation>& op) {
	TokenData::Data outData;
	if (ExpectToken(context, Token::BlendOperationValue, outData, "Expected a blend operation")) {
		op = outData.blendOperation;
		return true;
	}

	return false;
}

static bool ExpectBlendFactor(ParseContext& context, std::optional<Grindstone::GraphicsAPI::BlendFactor>& factor) {
	TokenData::Data outData;
	if (ExpectToken(context, Token::BlendFactorValue, outData, "Expected a blend factor")) {
		factor = outData.blendFactor;
		return true;
	}

	return false;
}

static void ExpectBlend(
	ParseContext& context,
	std::optional<Grindstone::GraphicsAPI::BlendOperation>& op,
	std::optional<Grindstone::GraphicsAPI::BlendFactor>& src,
	std::optional<Grindstone::GraphicsAPI::BlendFactor>& dst
) {
	if (ExpectBlendOp(context, op)) {
		if (ExpectBlendFactor(context, src)) {
			ExpectBlendFactor(context, dst);
		}
	}
}

static std::string GetEscapedSlashes(const std::filesystem::path& path) {
	std::string outStr = path.string();
	std::replace(outStr.begin(), outStr.end(), '\\', '/');
	return outStr;
}

static void ParseShader(ParseContext& context, ParseTree::ShaderBlock& selectedShaderBlock, ShaderCodeType shaderCodeType) {
	TokenData& token = context.scannerTokens[context.tokenIterator++];
	std::string prefix = fmt::format("#line {} \"{}\"", token.line, GetEscapedSlashes(token.path));
	std::string_view code = token.data.string;

	if (selectedShaderBlock.type == ShaderCodeType::Unset) {
		selectedShaderBlock.type = shaderCodeType;
		selectedShaderBlock.code += prefix;
		selectedShaderBlock.code += code;
	}
	else if (selectedShaderBlock.type == shaderCodeType) {
		selectedShaderBlock.code += prefix;
		selectedShaderBlock.code += code;
	}
	else {
		context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, "Shaderblock changed code type!", token.path, token.line, token.column);
	}
}

static bool ParseProperty(ParseContext& context, ParseTree::RenderState& renderState) {
	TokenData& token = context.scannerTokens[context.tokenIterator];

	switch (token.token) {
	case Token::CullModeKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'cull'")) {
			TokenData::Data outData;
			if (ExpectToken(context, Token::CullModeValue, outData, "Expected cull mode after 'cull:'")) {
				renderState.cullMode = outData.cullMode;
			}
		}

		return true;
	case Token::DepthBiasKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'depthBias'")) {
			TokenData::Data outData;
			if (ExpectToken(context, Token::Boolean, outData)) {
				renderState.isDepthBiasEnabled = outData.boolean;
			}
			else {
				std::optional<float> depthBiasConstantFactor, depthBiasSlopeFactor, depthBiasClamp;
				if (ExpectToken(context, Token::Number, outData)) {
					depthBiasConstantFactor = outData.number;
					if (ExpectToken(context, Token::Comma, outData)) {
						if (ExpectToken(context, Token::Number, outData)) {
							depthBiasSlopeFactor = outData.number;
							if (ExpectToken(context, Token::Comma, outData)) {
								if (ExpectToken(context, Token::Number, outData)) {
									depthBiasClamp = outData.number;
								}
							}
						}
					}
					else if (ExpectToken(context, Token::Number, outData)) {
						depthBiasSlopeFactor = outData.number;
						if (ExpectToken(context, Token::Number, outData)) {
							depthBiasClamp = outData.number;
						}
					}
				}

				renderState.isDepthBiasEnabled = true;

				if (depthBiasConstantFactor.has_value()) {
					renderState.depthBiasConstantFactor = depthBiasConstantFactor;
				}

				if (depthBiasSlopeFactor.has_value()) {
					renderState.depthBiasSlopeFactor = depthBiasSlopeFactor;
				}

				if (depthBiasClamp.has_value()) {
					renderState.depthBiasClamp = depthBiasClamp;
				}
			}
		}

		return true;
	case Token::DepthWriteKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'depthWrite'")) {
			bool outBool;
			if (ExpectBoolean(context, outBool, "Expected a boolean after 'depthWrite:'")) {
				renderState.isDepthWriteEnabled = outBool;
			}
		}

		return true;
	case Token::DepthTestKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'depthTest'")) {
			bool outBool;
			if (ExpectBoolean(context, outBool, "Expected a boolean after 'depthTest:'")) {
				renderState.isDepthTestEnabled = outBool;
			}
		}

		return true;
	case Token::DepthClampKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'depthClamp'")) {
			bool outBool;
			if (ExpectBoolean(context, outBool, "Expected a boolean after 'depthClamp:'")) {
				renderState.isDepthClampEnabled = outBool;
			}
		}

		return true;

	case Token::DepthCompareOpKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'depthCompareOp'")) {
			std::optional<Grindstone::GraphicsAPI::CompareOperation> depthCompareOp;
			ExpectCompareOp(context, depthCompareOp);
			if (depthCompareOp.has_value()) {
				renderState.depthCompareOp = depthCompareOp;
			}
		}

		return true;

	case Token::GeometryTypeKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'geometry'")) {
			std::optional<Grindstone::GraphicsAPI::GeometryType> geometryType;
			ExpectGeometryType(context, geometryType);
			if (geometryType.has_value()) {
				renderState.geometryType = geometryType;
			}
		}

		return true;

	case Token::FillModeKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'fill'")) {
			std::optional<Grindstone::GraphicsAPI::PolygonFillMode> fillMode;
			ExpectFillMode(context, fillMode);
			if (fillMode.has_value()) {
				renderState.polygonFillMode = fillMode;
			}
		}

		return true;
	}

	return false;
}

static bool ParseAttachmentProperty(ParseContext& context, ParseTree::RenderState::AttachmentData& attachment) {
	TokenData& token = context.scannerTokens[context.tokenIterator];

	switch (token.token) {
	case Token::ColorMask:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'colorMask'")) {
			std::string_view mask;

			if (ExpectTokenWithString(context, Token::Identifier, mask, "Expected an identifier such as 'rgba' after 'cull:'")) {
				Grindstone::GraphicsAPI::ColorMask colorMask = Grindstone::GraphicsAPI::ColorMask::None;
				bool isValid = true;

				for (char c : mask) {
					switch (c) {
					case 'r':
					case 'R':
						colorMask |= Grindstone::GraphicsAPI::ColorMask::Red;
						break;
					case 'g':
					case 'G':
						colorMask |= Grindstone::GraphicsAPI::ColorMask::Green;
						break;
					case 'b':
					case 'B':
						colorMask |= Grindstone::GraphicsAPI::ColorMask::Blue;
						break;
					case 'a':
					case 'A':
						colorMask |= Grindstone::GraphicsAPI::ColorMask::Alpha;
						break;
					default:
						isValid = false;
						TokenData& tokenData = context.scannerTokens[context.tokenIterator - 1];
						std::string invalidValue = "Expected only a combination of 'r', 'g', 'b', 'a' in 'colorMask' - found " + std::string(mask);
						context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, invalidValue, tokenData.path, tokenData.line, tokenData.column);
						break;
					}
				}

				attachment.colorMask = colorMask;
			}
		}

		return true;

	case Token::BlendPresetKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'blendPreset'")) {
			TokenData::Data outData;
			if (ExpectToken(context, Token::BlendPresetValue, outData, "Expected a blend preset (opaque, translucent, additive, multiplicative, or premultiply)")) {
				Grindstone::GraphicsAPI::BlendOperation colorOp, alphaOp;
				Grindstone::GraphicsAPI::BlendFactor colorSrc, colorDst, alphaSrc, alphaDst;

				switch (outData.blendPreset) {
				case BlendPreset::Opaque:
					colorOp = Grindstone::GraphicsAPI::BlendOperation::None;
					alphaOp = Grindstone::GraphicsAPI::BlendOperation::None;
					colorSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					colorDst = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaDst = Grindstone::GraphicsAPI::BlendFactor::One;
					break;
				case BlendPreset::Translucent:
					colorOp = Grindstone::GraphicsAPI::BlendOperation::Add;
					alphaOp = Grindstone::GraphicsAPI::BlendOperation::Add;
					colorSrc = Grindstone::GraphicsAPI::BlendFactor::SrcAlpha;
					colorDst = Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcAlpha;
					alphaSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaDst = Grindstone::GraphicsAPI::BlendFactor::Zero;
					break;
				case BlendPreset::Additive:
					colorOp = Grindstone::GraphicsAPI::BlendOperation::Add;
					alphaOp = Grindstone::GraphicsAPI::BlendOperation::Add;
					colorSrc = Grindstone::GraphicsAPI::BlendFactor::SrcAlpha;
					colorDst = Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcAlpha;
					alphaSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaDst = Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcAlpha;
					break;
				case BlendPreset::Multiplicative:
					colorOp = Grindstone::GraphicsAPI::BlendOperation::Multiply;
					alphaOp = Grindstone::GraphicsAPI::BlendOperation::Multiply;
					colorSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					colorDst = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaDst = Grindstone::GraphicsAPI::BlendFactor::One;
					break;
				case BlendPreset::Premultiply:
					colorOp = Grindstone::GraphicsAPI::BlendOperation::Add;
					alphaOp = Grindstone::GraphicsAPI::BlendOperation::Add;
					colorSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					colorDst = Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcAlpha;
					alphaSrc = Grindstone::GraphicsAPI::BlendFactor::One;
					alphaDst = Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcAlpha;
					break;
				}

				attachment.blendColorOperation = colorOp;
				attachment.blendAlphaOperation = alphaOp;
				attachment.blendColorFactorSrc = colorSrc;
				attachment.blendColorFactorDst = colorDst;
				attachment.blendAlphaFactorSrc = alphaSrc;
				attachment.blendAlphaFactorDst = alphaDst;
			}
		}

		return true;
	case Token::BlendColorKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'blendColor'")) {
			ExpectBlend(
				context,
				attachment.blendColorOperation,
				attachment.blendColorFactorSrc,
				attachment.blendColorFactorDst
			);
		}

		return true;
	case Token::BlendAlphaKey:
		++context.tokenIterator;
		if (ExpectColon(context, "Expected a colon after 'blendAlpha'")) {
			ExpectBlend(
				context,
				attachment.blendAlphaOperation,
				attachment.blendAlphaFactorSrc,
				attachment.blendAlphaFactorDst
			);
		}

		return true;
	}

	return false;
}

static void ParseAttachments(ParseContext& context, ParseTree::RenderState& renderState) {
	TokenData& attachmentsTokenData = context.scannerTokens[context.tokenIterator];

	ExpectColon(context, "Expected a colon ':' after \"attachments\"");

	if (ExpectToken(context, Token::OpenCurly)) {
		ParseTree::RenderState::AttachmentData attachmentData;
		while (true) {
			if (ExpectToken(context, Token::CloseCurly)) {
				break;
			}
			else if (!ParseAttachmentProperty(context, attachmentData)) {
				UnexpectedError(context);
				++context.tokenIterator;
			}
		}

		renderState.attachmentData.resize(1);
		renderState.shouldCopyFirstAttachment = true;
		renderState.attachmentData[0] = attachmentData;
	}
	else if (ExpectToken(context, Token::OpenSquareBrace)) {
		std::vector<ParseTree::RenderState::AttachmentData> attachments;

		size_t attachmentIndex = 0;
		while (true) {
			if (ExpectToken(context, Token::CloseSquareBrace)) {
				break;
			}
			else if (ExpectToken(context, Token::OpenCurly)) {
				attachments.emplace_back();
				if (attachmentIndex == MAXIMUM_ATTACHMENT_COUNT) {
					TokenData commaToken = context.scannerTokens[context.tokenIterator];
					context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, "Too many attachments!", commaToken.path, commaToken.line, commaToken.column);
					break;
				}

				while (true) {
					if (ExpectToken(context, Token::CloseCurly)) {
						break;
					}
					else if (!ParseAttachmentProperty(context, attachments.back())) {
						UnexpectedError(context);
						++context.tokenIterator;
					}
				}

				if (ExpectToken(context, Token::Comma)) {
					continue;
				}
				else if (ExpectToken(context, Token::CloseSquareBrace)) {
					break;
				}
				else {
					UnexpectedError(context);
					++context.tokenIterator;
					break;
				}
			}
			else {
				UnexpectedError(context);
				++context.tokenIterator;
				break;
			}
		}

		renderState.shouldCopyFirstAttachment = false;
		renderState.attachmentData.resize(attachments.size());
		for (size_t i = 0; i < attachments.size(); ++i) {
			renderState.attachmentData[i] = attachments[i];
		}
	}
	else {
		context.Log(
			Grindstone::LogSeverity::Error,
			PipelineConverterLogSource::Parser,
			"Expected an opening curly brace '{' or square brace '[' after \"attachments:\".",
			attachmentsTokenData.path,
			attachmentsTokenData.line,
			attachmentsTokenData.column
		);
	}
}

static void ParseRenderState(ParseContext& context, ParseTree::Pass& pass) {
	++context.tokenIterator;

	if (!ExpectToken(context, Token::OpenCurly, "Expected an open curly brace '{'")) {
		return;
	}

	ParseTree::RenderState& renderState = pass.renderState;

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		if (token.token == Token::CloseCurly) {
			++context.tokenIterator;
			break;
		}
		else if (ExpectToken(context, Token::Attachments)) {
			ParseAttachments(context, renderState);
		}
		else if (!ParseProperty(context, renderState)) {
			UnexpectedError(context, token);
			++context.tokenIterator;
		}
	}
}

static void ParseRequiresBlocks(ParseContext& context, std::vector<std::string>& names) {
	++context.tokenIterator;

	ExpectToken(context, Token::OpenSquareBrace, "Expected an open square brace '['");

	bool lastWasString = false;
	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::CloseSquareBrace:
			++context.tokenIterator;
			return;
		case Token::Identifier:
			++context.tokenIterator;
			if (lastWasString) {
				context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, "Expected a comma before another identifier!", token.path, token.line, token.column);
			}
			lastWasString = true;
			names.push_back(std::string(token.data.string));
			break;
		case Token::Comma:
			++context.tokenIterator;
			if (!lastWasString) {
				context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, "Expected an identifier before a comma!", token.path, token.line, token.column);
			}
			lastWasString = false;
			break;
		default:
			UnexpectedError(context, token);
			++context.tokenIterator;
			break;
		}
	}
}

template<typename T>
static T* HandleObjectDeclaration(ParseContext& context, std::map<std::string, T>& map) {
	bool isAbstract = false;
	if (ExpectToken(context, Token::Abstract)) {
		isAbstract = true;
	}

	std::string_view name;
	if (!ExpectTokenWithString(context, Token::String, name)) {
		UnexpectedError(context);
		return nullptr;
	}

	T& newValue = map[std::string(name)];
	newValue.isAbstract = isAbstract;
	newValue.sourceFilepath = context.filepath;

	std::string_view parent;
	if (ExpectToken(context, Token::Inherits)) {
		if (ExpectTokenWithString(context, Token::String, parent)) {
			newValue.parentData.parent = parent;
			newValue.parentData.parentType = ParseTree::ParentType::Inherit;
		}
		else {
			UnexpectedError(context);
		}
	}
	else if (ExpectToken(context, Token::Clones)) {
		if (ExpectTokenWithString(context, Token::String, parent)) {
			newValue.parentData.parent = parent;
			newValue.parentData.parentType = ParseTree::ParentType::Clone;
		}
		else {
			UnexpectedError(context);
		}
	}

	ExpectToken(context, Token::OpenCurly, "Expected an open curly brace '{'");
	return &newValue;
}

static void ParseShaderEntrypoint(ParseContext& context, std::array<std::string, Grindstone::GraphicsAPI::numShaderTotalStage>& entrypoints) {
	++context.tokenIterator;
	if (ExpectColon(context, "Expected a colon after 'shaderEntrypoint'")) {
		TokenData::Data parseData;
		if (ExpectToken(context, Token::StageValue, parseData, "Expected a stage after 'shaderEntrypoint:'")) {
			std::string_view outStr;
			if (ExpectTokenWithString(context, Token::Identifier, outStr, "Expected an identifer after 'shaderEntrypoint: {stage}'")) {
				uint8_t stageIndex = static_cast<uint8_t>(parseData.shaderStage);
				entrypoints[stageIndex] = outStr;
				return;
			}
		}
	}

	UnexpectedError(context);
}

static void ParseShaderBlock(ParseContext& context) {
	++context.tokenIterator;

	std::string_view name;
	if (!ExpectTokenWithString(context, Token::Identifier, name)) {
		return;
	}

	ParseTree::ShaderBlock& shaderBlock = context.parseTree.genericShaderBlocks[std::string(name)];

	ExpectToken(context, Token::OpenCurly, "Expected an open curly brace '{'");

	std::vector<std::string>& requiredBlocks = shaderBlock.requiredShaderBlocks;

	bool lastWasString = false;
	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::CloseCurly:
			++context.tokenIterator;
			return;
		case Token::ShaderEntrypoint:
			ParseShaderEntrypoint(context, shaderBlock.stageEntryPoints);
			break;
		case Token::ShaderBlock:
			ParseShaderBlock(context);
			break;
		case Token::RequiresBlocks:
			ParseRequiresBlocks(context, requiredBlocks);
			break;
		case Token::ShaderGlsl:
			ParseShader(context, shaderBlock, ShaderCodeType::Glsl);
			break;
		case Token::ShaderHlsl:
			ParseShader(context, shaderBlock, ShaderCodeType::Hlsl);
			break;
		default:
			UnexpectedError(context, token);
			++context.tokenIterator;
			break;
		}
	}
}

static bool ParsePass(ParseContext& context, ParseTree::Configuration* parentConfig) {
	++context.tokenIterator;

	std::map<std::string, ParseTree::Pass>& passMap = parentConfig != nullptr
		? parentConfig->passes
		: context.parseTree.genericPasses;

	ParseTree::Pass* pass = HandleObjectDeclaration(context, passMap);
	if (pass == nullptr) {
		return false;
	}

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::CloseCurly:
			++context.tokenIterator;
			return true;
		case Token::ShaderEntrypoint:
			ParseShaderEntrypoint(context, pass->shaderBlock.stageEntryPoints);
			break;
		case Token::ShaderBlock:
			ParseShaderBlock(context);
			break;
		case Token::Properties:
			ParseRenderState(context, *pass);
			break;
		case Token::RequiresBlocks:
			ParseRequiresBlocks(context, pass->shaderBlock.requiredShaderBlocks);
			break;
		case Token::ShaderGlsl:
			ParseShader(context, pass->shaderBlock, ShaderCodeType::Glsl);
			break;
		case Token::ShaderHlsl:
			ParseShader(context, pass->shaderBlock, ShaderCodeType::Hlsl);
			break;
		case Token::RenderQueue: {
			++context.tokenIterator;
			ExpectColon(context, "Expected a colon after renderQueue.");
			std::string_view renderQueue;
			ExpectTokenWithString(context, Token::String, renderQueue);
			pass->renderQueue = renderQueue;
			break;
		}
		default:
			UnexpectedError(context, token);
			++context.tokenIterator;
			break;
		}
	}

	return true;
}

static bool ParseConfiguration(ParseContext& context, ParseTree::PipelineSet* pipelineSet) {
	++context.tokenIterator;

	std::map<std::string, ParseTree::Configuration>& configMap = pipelineSet != nullptr
		? pipelineSet->configurations
		: context.parseTree.genericConfigurations;

	ParseTree::Configuration* config = HandleObjectDeclaration(context, configMap);
	if (config == nullptr) {
		return false;
	}

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::CloseCurly:
			++context.tokenIterator;
			return true;
		case Token::Pass:
			ParsePass(context, config);
			break;
		case Token::RendererTags:
			++context.tokenIterator;
			
			if (ExpectColon(context, "Expected a colon ':' after 'rendererTags:'")) {
				std::string_view outData;
				while (ExpectTokenWithString(context, Token::String, outData)) {
					config->tags.push_back(outData);
				}
			}
			break;
		default:
			UnexpectedError(context, token);
			++context.tokenIterator;
			break;
		}
	}

	return true;
}

static bool ReadParameter(ParseContext& context, ParseTree::PipelineSet& pipelineSet, TokenData& token) {
	ParameterType parameterType;
	if (!ExpectParameter(context, parameterType)) {
		return false;
	}

	std::string_view identifier;
	ExpectTokenWithString(context, Token::Identifier, identifier);

	ExpectColon(context, "Expected a colon ':' between identifier and default value.");

	std::string_view defaultValue;
	ExpectTokenWithString(context, Token::Identifier, defaultValue);

	pipelineSet.parameters.emplace_back(
		ParseTree::MaterialParameter{
			parameterType,
			std::string(identifier),
			std::string(defaultValue)
		}
	);

	return true;
}

static bool ParseParameters(ParseContext& context, ParseTree::PipelineSet& pipelineSet) {
	++context.tokenIterator;

	ExpectToken(context, Token::OpenCurly, "Expected openingcurly brace");

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		bool requiresComma = false;
		switch (token.token) {
		case Token::CloseCurly:
			++context.tokenIterator;
			return false;
		case Token::Comma:
			++context.tokenIterator;
			requiresComma = false;
			break;
		case Token::Parameter:
			if (requiresComma == true) {
				context.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Parser, "Expected a comma before this identifier", token.path, token.line, token.column);
			}
			
			ReadParameter(context, pipelineSet, token);
			requiresComma = true;
			
			break;
		default:
			++context.tokenIterator;
			UnexpectedError(context, token);
			break;
		}
	}

	return true;
}

static void ParsePipelineSet(ParseContext& context) {
	++context.tokenIterator;

	ParseTree::PipelineSet* pipelineSet = HandleObjectDeclaration(context, context.parseTree.pipelineSets);
	if (pipelineSet == nullptr) {
		return;
	}

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::CloseCurly:
			++context.tokenIterator;
			return;
		case Token::Parameters:
			ParseParameters(context, *pipelineSet);
			break;
		case Token::Configuration:
			ParseConfiguration(context, pipelineSet);
			break;
		default:
			++context.tokenIterator;
			UnexpectedError(context, token);
			break;
		}
	}
}

static void ParseComputeSet(ParseContext& context) {
	++context.tokenIterator;

	std::string_view name;
	if (!ExpectTokenWithString(context, Token::String, name)) {
		UnexpectedError(context);
		return;
	}

	ParseTree::ComputeSet& computeSet = context.parseTree.computeSets[std::string(name)];

	ExpectToken(context, Token::OpenCurly, "Expected an open curly brace '{'");

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::CloseCurly:
			++context.tokenIterator;
			return;
		case Token::ShaderEntrypoint:
			ParseShaderEntrypoint(context, computeSet.shaderBlock.stageEntryPoints);
			break;
		case Token::RequiresBlocks:
			ParseRequiresBlocks(context, computeSet.shaderBlock.requiredShaderBlocks);
			break;
		case Token::ShaderGlsl:
			ParseShader(context, computeSet.shaderBlock, ShaderCodeType::Glsl);
			break;
		case Token::ShaderHlsl:
			ParseShader(context, computeSet.shaderBlock, ShaderCodeType::Hlsl);
			break;
		default:
			UnexpectedError(context, token);
			++context.tokenIterator;
			break;
		}
	}
}

bool ParsePipelineSet(LogCallback logFn, const std::filesystem::path& path, TokenList& scannerTokens, ParseTree& parseTree, std::set<std::filesystem::path>& unprocessedFiles) {
	ParseContext context(scannerTokens, path, logFn, parseTree);

	while (context.tokenIterator < context.scannerTokens.size()) {
		TokenData& token = context.scannerTokens[context.tokenIterator];

		switch (token.token) {
		case Token::Include: {
			++context.tokenIterator;

			std::string_view includePath;
			if (ExpectTokenWithString(context, Token::String, includePath, "Expected a string after the keyword 'include'.")) {
				unprocessedFiles.insert(includePath);
			}
			break;
		}
		case Token::PipelineSet:
			ParsePipelineSet(context);
			break;
		case Token::ComputeSet:
			ParseComputeSet(context);
			break;
		case Token::ShaderBlock:
			ParseShaderBlock(context);
			break;
		case Token::Configuration:
			ParseConfiguration(context, nullptr);
			break;
		case Token::Pass:
			ParsePass(context, nullptr);
			break;
		default:
			++context.tokenIterator;
			UnexpectedError(context, token);
			break;
		}
	}

	return true;
}
