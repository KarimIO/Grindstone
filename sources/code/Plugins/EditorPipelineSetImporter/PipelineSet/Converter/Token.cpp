#include "Token.hpp"
#include "Keyword.hpp"

TokenData::TokenData(Token token, const std::filesystem::path& path, uint32_t line, uint32_t column) noexcept : token(token), data(), path(path), line(line), column(column) {}
TokenData::TokenData(Token token, Data data, const std::filesystem::path& path, uint32_t line, uint32_t column) noexcept : token(token), data(data), path(path), line(line), column(column) {}

TokenData::TokenData(const TokenData& other) noexcept : token(other.token), data(other.data), path(other.path), line(other.line), column(other.column) {}
TokenData::TokenData(TokenData&& other) noexcept : token(other.token), data(other.data), path(other.path), line(other.line), column(other.column) {}

TokenData TokenData::CommentMultiLine(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::CommentMultiLine, string, path, line, column);
}

TokenData TokenData::Comment(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::Comment, string, path, line, column);
}

TokenData TokenData::Identifier(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::Identifier, string, path, line, column);
}

TokenData TokenData::String(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::String, string, path, line, column);
}

TokenData TokenData::Boolean(bool boolean, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::Boolean, boolean, path, line, column);
}

TokenData TokenData::Number(float number, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::Number, number, path, line, column);
}

TokenData TokenData::ShaderGlsl(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::ShaderGlsl, string, path, line, column);
}

TokenData TokenData::ShaderHlsl(std::string_view string, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::ShaderHlsl, string, path, line, column);
}

TokenData TokenData::GeometryTypeValue(Grindstone::GraphicsAPI::GeometryType mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::GeometryTypeValue, mode, path, line, column);
}

TokenData TokenData::FillModeValue(Grindstone::GraphicsAPI::PolygonFillMode mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::FillModeValue, mode, path, line, column);
}

TokenData TokenData::CullModeValue(Grindstone::GraphicsAPI::CullMode mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::CullModeValue, mode, path, line, column);
}

TokenData TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::CompareOperationValue, mode, path, line, column);
}

TokenData TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::BlendOperationValue, mode, path, line, column);
}

TokenData TokenData::BlendPresetValue(BlendPreset preset, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::BlendPresetValue, preset, path, line, column);
}

TokenData TokenData::Parameter(ParameterType parameterType, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::Parameter, parameterType, path, line, column);
}

TokenData TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::BlendFactorValue, mode, path, line, column);
}

TokenData TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage mode, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	return TokenData(Token::StageValue, mode, path, line, column);
}

std::map<std::string_view, Token> stringToTokenMap {
#define E(val) { #val, Token::val },
	TOKEN_LIST
#undef E
};

std::map<std::string_view, Keyword> stringToKeywordMap{
#define E(val) { #val, Keyword::val },
	KEYWORD_LIST
#undef E
	{ "include", Keyword::includeKeyword },
	{ "abstract", Keyword::abstractKeyword },
	{ "true", Keyword::boolTrue },
	{ "false", Keyword::boolFalse },
};
