#pragma once

#include <string_view>
#include <filesystem>
#include <vector>
#include <map>

#include <Common/Graphics/Formats.hpp>

#define TOKEN_LIST \
E(Invalid)\
E(EndOfFile)\
E(Include)\
E(Parameters)\
E(Abstract)\
E(ComputeSet)\
E(PipelineSet)\
E(Clones)\
E(Inherits)\
E(ShaderBlock)\
E(RequiresBlocks)\
E(CommentMultiLine)\
E(Comment)\
E(Identifier)\
E(String)\
E(Number)\
E(Boolean)\
E(Colon)\
E(SemiColon)\
E(Comma)\
E(OpenSquareBrace)\
E(CloseSquareBrace)\
E(OpenCurly)\
E(CloseCurly)\
E(OpenParenthesis)\
E(CloseParenthesis)\
E(ShaderHlsl)\
E(ShaderGlsl)\
E(Name)\
E(Properties)\
E(Configuration)\
E(Pass)\
E(StageKey)\
E(ShaderEntrypoint)\
E(RendererTags)\
E(PassTags)\
E(Attachments)\
E(ColorMask)\
E(PrimitiveTypeKey)\
E(FillModeKey)\
E(CullModeKey)\
E(DepthBiasKey)\
E(DepthWriteKey)\
E(DepthTestKey)\
E(DepthClampKey)\
E(DepthCompareOpKey)\
E(BlendColorKey)\
E(BlendAlphaKey)\
E(StageValue)\
E(PrimitiveTypeValue)\
E(FillModeValue)\
E(CullModeValue)\
E(CompareOperationValue)\
E(BlendOperationValue)\
E(BlendFactorValue)\
E(BlendPresetKey)\
E(BlendPresetValue)

enum class Token : uint8_t {
#define E(val) val,
	TOKEN_LIST
#undef E
};

constexpr const char* tokenStrings[] = {
#define E(val) #val,
	TOKEN_LIST
#undef E
};

enum class BlendPreset : uint8_t {
	Opaque,
	Translucent,
	Additive,
	Multiplicative,
	Premultiply
};

class TokenData {
public:
	union Data {
		std::string_view string;
		bool boolean;
		float number;
		Grindstone::GraphicsAPI::GeometryType primitiveType;
		Grindstone::GraphicsAPI::PolygonFillMode polygonFillMode;
		Grindstone::GraphicsAPI::CullMode cullMode;
		Grindstone::GraphicsAPI::CompareOperation compareOperation;
		Grindstone::GraphicsAPI::BlendOperation blendOperation;
		Grindstone::GraphicsAPI::BlendFactor blendFactor;
		Grindstone::GraphicsAPI::ShaderStage shaderStage;
		BlendPreset blendPreset;

		Data() : boolean(false) {}
		Data(bool boolean) : boolean(boolean) {}
		Data(float number) : number(number) {}
		Data(std::string_view string) : string(string) {}
		Data(Grindstone::GraphicsAPI::CullMode cullMode) : cullMode(cullMode) {}
		Data(Grindstone::GraphicsAPI::CompareOperation compareOperation) : compareOperation(compareOperation) {}
		Data(Grindstone::GraphicsAPI::BlendOperation blendOperation) : blendOperation(blendOperation) {}
		Data(Grindstone::GraphicsAPI::BlendFactor blendFactor) : blendFactor(blendFactor) {}
		Data(Grindstone::GraphicsAPI::GeometryType primitiveType) : primitiveType(primitiveType) {}
		Data(Grindstone::GraphicsAPI::PolygonFillMode polygonFillMode) : polygonFillMode(polygonFillMode) {}
		Data(Grindstone::GraphicsAPI::ShaderStage shaderStage) : shaderStage(shaderStage) {}
		Data(BlendPreset blendPreset) : blendPreset(blendPreset) {}
	};

	Token token;
	Data data;
	const std::filesystem::path& path;
	uint32_t line;
	uint32_t column;

	TokenData(Token token, const std::filesystem::path& path, uint32_t line, uint32_t column) noexcept;
	TokenData(Token token, Data data, const std::filesystem::path& path, uint32_t line, uint32_t column) noexcept;
	TokenData(const TokenData& other) noexcept;
	TokenData(TokenData&& other) noexcept;

	static TokenData CommentMultiLine(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData Comment(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData Identifier(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData String(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData Boolean(bool boolean, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData Number(float number, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData ShaderGlsl(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData ShaderHlsl(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData FillModeValue(Grindstone::GraphicsAPI::PolygonFillMode mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData CullModeValue(Grindstone::GraphicsAPI::CullMode mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
	static TokenData BlendPresetValue(BlendPreset mode, const std::filesystem::path& path, uint32_t line, uint32_t column);
};

using TokenList = std::vector<TokenData>;

extern std::map<std::string_view, Token> stringToTokenMap;

