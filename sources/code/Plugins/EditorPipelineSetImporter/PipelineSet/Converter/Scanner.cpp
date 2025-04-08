#include <iostream>
#include <cctype>

#include "Scanner.hpp"
#include "Keyword.hpp"

static void MoveAppend(std::vector<TokenData>& src, std::vector<TokenData>& dst) {
	if (dst.empty()) {
		dst = std::move(src);
	}
	else {
		dst.reserve(dst.size() + src.size());
		std::move(std::begin(src), std::end(src), std::back_inserter(dst));
		src.clear();
	}
}

struct ScanContext {
	uint32_t currentLine = 1;
	uint32_t currentCharacterInLine = 1;
	std::string_view content;
	std::string_view::const_iterator currentPtr;
	const std::filesystem::path& path;
	LogCallback Log = nullptr;
	bool hasError = false;

	ScanContext(std::string_view content, const std::filesystem::path& path, LogCallback logFn) : content(content), currentPtr(content.begin()), path(path), Log(logFn) {}
};

static void SkipWhitespace(ScanContext& scanContext) {
	while (scanContext.currentPtr < scanContext.content.end()) {
		if (!std::isspace(*scanContext.currentPtr)) {
			break;
		}
		else if (*scanContext.currentPtr == '\n') {
			++scanContext.currentLine;
			scanContext.currentCharacterInLine = 0;
			++scanContext.currentPtr;
		}
		else {
			++scanContext.currentCharacterInLine;
			++scanContext.currentPtr;
		}
	}
}

// TODO: Handle escape character
static std::string_view ReadString(ScanContext& scanContext) {
	const char* start = &*scanContext.currentPtr;

	while (scanContext.currentPtr < scanContext.content.end()) {
		if (*scanContext.currentPtr == '"') {
			++scanContext.currentLine;
			++scanContext.currentPtr;

			const char* end = &*scanContext.currentPtr - 1;
			return std::string_view(start, end - start);
		}
		if (*scanContext.currentPtr == '\n') {
			++scanContext.currentLine;
			scanContext.currentCharacterInLine = 0;
			++scanContext.currentPtr;
		}
		else {
			++scanContext.currentCharacterInLine;
			++scanContext.currentPtr;
		}
	}

	const char* end = &*scanContext.currentPtr;
	return std::string_view(start, end - start);
}

static std::string_view ReadShaderCode(ScanContext& scanContext) {
	SkipWhitespace(scanContext);

	int32_t scopeCount = 0;
	const char* start = nullptr;

	while (scanContext.currentPtr < scanContext.content.end()) {
		if (*scanContext.currentPtr == '\n') {
			++scanContext.currentLine;
			scanContext.currentCharacterInLine = 0;
			++scanContext.currentPtr;
		}
		else if (*scanContext.currentPtr == '{') {
			++scanContext.currentCharacterInLine;
			++scanContext.currentPtr;

			if (++scopeCount == 1) {
				start = &*scanContext.currentPtr;
			}
		}
		else if (*scanContext.currentPtr == '}') {
			++scanContext.currentCharacterInLine;
			++scanContext.currentPtr;

			if (--scopeCount == 0) {
				const char* end = &*scanContext.currentPtr - 1;
				return std::string_view(start, end - start);
			}
		}
		else {
			++scanContext.currentCharacterInLine;
			++scanContext.currentPtr;
		}
	}

	if (start == nullptr) {
		return "No shader";
	}

	const char* end = &*scanContext.currentPtr;
	return std::string_view(start, end - start);
}

static std::string_view ReadCommentSingleLine(ScanContext& scanContext) {
	scanContext.currentCharacterInLine += 2;
	scanContext.currentPtr += 2;

	const char* start = &*scanContext.currentPtr;

	while (scanContext.currentPtr < scanContext.content.end()) {
		if (*scanContext.currentPtr == '\n') {
			const char* end = &*scanContext.currentPtr;

			++scanContext.currentLine;
			scanContext.currentCharacterInLine = 0;
			++scanContext.currentPtr;

			return std::string_view(start, end - start);
		}
		else {
			++scanContext.currentCharacterInLine;
			++scanContext.currentPtr;
		}
	}

	const char* end = &*scanContext.currentPtr;
	return std::string_view(start, end - start);
}

static std::string_view ReadCommentMultiLine(ScanContext& scanContext) {
	scanContext.currentCharacterInLine += 2;
	scanContext.currentPtr += 2;

	const char* start = &*scanContext.currentPtr;
	bool foundStar = false;

	while (scanContext.currentPtr < scanContext.content.end()) {
		char c = *scanContext.currentPtr;

		++scanContext.currentPtr;
		++scanContext.currentCharacterInLine;

		if (c == '*') {
			foundStar = true;
		}
		else if (c == '\n') {
			++scanContext.currentLine;
			scanContext.currentCharacterInLine = 1;
			foundStar = false;
		}
		else if (foundStar && c == '/') {
			const char* end = &*(scanContext.currentPtr - 2);
			return std::string_view(start, end - start);
		}
		else {
			foundStar = false;
		}
	}

	scanContext.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Scanner, "Unended comment!", scanContext.path, scanContext.currentLine, scanContext.currentCharacterInLine);
	const char* end = &*(scanContext.currentPtr - 1);
	return std::string_view(start, end - start);
}

static bool ReadNumber(ScanContext& scanContext, float& number) {
	bool digit = false;
	bool point = false;
	unsigned divide = 1;
	number = 0.0;

	while (scanContext.currentPtr < scanContext.content.end()) {
		char c = *scanContext.currentPtr;
		if (c >= '0' && c <= '9') {
			digit = true;

			char newnum = (c - '0');
			number = 10 * number + newnum;

			if (point) {
				divide *= 10;
			}
		}
		else if (c == '.') {
			point = true;
		}
		else {
			break;
		}

		++scanContext.currentPtr;
		++scanContext.currentCharacterInLine;
	}

	if (digit) {
		number /= divide;
		return true;
	}

	scanContext.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Scanner, "Invalid Number!", scanContext.path, scanContext.currentLine, scanContext.currentCharacterInLine);
	return false;
}

static std::string_view ReadIdentifier(ScanContext& scanContext) {
	const char* start = &*scanContext.currentPtr;
	++scanContext.currentPtr;
	++scanContext.currentCharacterInLine;

	while (scanContext.currentPtr < scanContext.content.end()) {
		char c = *scanContext.currentPtr;

		if (!isalnum(c) && c != '.') {
			const char* end = &*scanContext.currentPtr;
			return std::string_view(start, end - start);
		}

		++scanContext.currentPtr;
		++scanContext.currentCharacterInLine;
	}

	const char* end = &*scanContext.currentPtr;
	return std::string_view(start, end - start);
}

static TokenData ParseKeywordToken(ScanContext& scanContext, Keyword keyword, const std::filesystem::path& path, uint32_t line, uint32_t column) {
	switch (keyword) {
	case Keyword::shaderHlsl:
		return TokenData::ShaderHlsl(ReadShaderCode(scanContext), path, line, column);
	case Keyword::shaderGlsl:
		return TokenData::ShaderGlsl(ReadShaderCode(scanContext), path, line, column);
	case Keyword::parameters:
		return TokenData(Token::Parameters, path, line, column);
	case Keyword::name:
		return TokenData(Token::Name, path, line, column);
	case Keyword::inherits:
		return TokenData(Token::Inherits, path, line, column);
	case Keyword::clones:
		return TokenData(Token::Clones, path, line, column);
	case Keyword::properties:
		return TokenData(Token::Properties, path, line, column);
	case Keyword::configuration:
		return TokenData(Token::Configuration, path, line, column);
	case Keyword::pass:
		return TokenData(Token::Pass, path, line, column);
	case Keyword::stage:
		return TokenData(Token::StageKey, path, line, column);
	case Keyword::pipelineSet:
		return TokenData(Token::PipelineSet, path, line, column);
	case Keyword::abstractKeyword:
		return TokenData(Token::Abstract, path, line, column);
	case Keyword::computeSet:
		return TokenData(Token::ComputeSet, path, line, column);
	case Keyword::shaderBlock:
		return TokenData(Token::ShaderBlock, path, line, column);
	case Keyword::requiresBlocks:
		return TokenData(Token::RequiresBlocks, path, line, column);
	case Keyword::shaderEntrypoint:
		return TokenData(Token::ShaderEntrypoint, path, line, column);
	case Keyword::rendererTags:
		return TokenData(Token::RendererTags, path, line, column);
	case Keyword::passTags:
		return TokenData(Token::PassTags, path, line, column);
	case Keyword::attachments:
		return TokenData(Token::Attachments, path, line, column);
	case Keyword::colorMask:
		return TokenData(Token::ColorMask, path, line, column);
	case Keyword::cull:
		return TokenData(Token::CullModeKey, path, line, column);
	case Keyword::depthBias:
		return TokenData(Token::DepthBiasKey, path, line, column);
	case Keyword::depthWrite:
		return TokenData(Token::DepthWriteKey, path, line, column);
	case Keyword::depthTest:
		return TokenData(Token::DepthTestKey, path, line, column);
	case Keyword::depthClamp:
		return TokenData(Token::DepthClampKey, path, line, column);
	case Keyword::depthCompareOp:
		return TokenData(Token::DepthCompareOpKey, path, line, column);
	case Keyword::blendColor:
		return TokenData(Token::BlendColorKey, path, line, column);
	case Keyword::blendAlpha:
		return TokenData(Token::BlendAlphaKey, path, line, column);
	case Keyword::blendPreset:
		return TokenData(Token::BlendPresetKey, path, line, column);

	// Primitive Types
	case Keyword::points:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::Points, path, line, column);
	case Keyword::lines:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::Lines, path, line, column);
	case Keyword::lineStrips:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::LineStrips, path, line, column);
	case Keyword::lineLoops:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::LineLoops, path, line, column);
	case Keyword::triangleStrips:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::TriangleStrips, path, line, column);
	case Keyword::triangleFans:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::TriangleFans, path, line, column);
	case Keyword::triangles:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::Triangles, path, line, column);
	case Keyword::linesAdjacency:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::LinesAdjacency, path, line, column);
	case Keyword::trianglesAdjacency:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::TrianglesAdjacency, path, line, column);
	case Keyword::triangleStripsAdjacency:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::TriangleStripsAdjacency, path, line, column);
	case Keyword::patches:
		return TokenData::PrimitiveTypeValue(Grindstone::GraphicsAPI::GeometryType::Patches, path, line, column);
	
	// Fill Modes
	case Keyword::point:
		return TokenData::FillModeValue(Grindstone::GraphicsAPI::PolygonFillMode::Point, path, line, column);
	case Keyword::line:
		return TokenData::FillModeValue(Grindstone::GraphicsAPI::PolygonFillMode::Line, path, line, column);
	case Keyword::fill:
		return TokenData::FillModeValue(Grindstone::GraphicsAPI::PolygonFillMode::Fill, path, line, column);
	
	// Compare Operation
	case Keyword::never:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::Never, path, line, column);
	case Keyword::less:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::Less, path, line, column);
	case Keyword::equal:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::Equal, path, line, column);
	case Keyword::lessOrEqual:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::LessOrEqual, path, line, column);
	case Keyword::greater:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::Greater, path, line, column);
	case Keyword::notEqual:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::NotEqual, path, line, column);
	case Keyword::greaterOrEqual:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::GreaterOrEqual, path, line, column);
	case Keyword::always:
		return TokenData::CompareOperationValue(Grindstone::GraphicsAPI::CompareOperation::Always, path, line, column);
	
	// Cull Mode
	case Keyword::none:
		return TokenData::CullModeValue(Grindstone::GraphicsAPI::CullMode::None, path, line, column);
	case Keyword::front:
		return TokenData::CullModeValue(Grindstone::GraphicsAPI::CullMode::Front, path, line, column);
	case Keyword::back:
		return TokenData::CullModeValue(Grindstone::GraphicsAPI::CullMode::Back, path, line, column);
	case Keyword::both:
		return TokenData::CullModeValue(Grindstone::GraphicsAPI::CullMode::Both, path, line, column);
	
	// Blend Presets
	case Keyword::opaque:
		return TokenData::BlendPresetValue(BlendPreset::Opaque, path, line, column);
	case Keyword::translucent:
		return TokenData::BlendPresetValue(BlendPreset::Translucent, path, line, column);
	case Keyword::additive:
		return TokenData::BlendPresetValue(BlendPreset::Additive, path, line, column);
	case Keyword::multiplicative:
		return TokenData::BlendPresetValue(BlendPreset::Multiplicative, path, line, column);
	case Keyword::premultiply:
		return TokenData::BlendPresetValue(BlendPreset::Premultiply, path, line, column);

	// Blend Factors
	case Keyword::zero:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::Zero, path, line, column);
	case Keyword::one:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::One, path, line, column);
	case Keyword::srcColor:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::SrcColor, path, line, column);
	case Keyword::oneMinusSrcColor:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcColor, path, line, column);
	case Keyword::dstColor:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::DstColor, path, line, column);
	case Keyword::oneMinusDstColor:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusDstColor, path, line, column);
	case Keyword::srcAlpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::SrcAlpha, path, line, column);
	case Keyword::oneMinusSrcAlpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusSrcAlpha, path, line, column);
	case Keyword::dstAlpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::DstAlpha, path, line, column);
	case Keyword::oneMinusDstAlpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusDstAlpha, path, line, column);
	case Keyword::constantColor:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::ConstantColor, path, line, column);
	case Keyword::oneMinusConstantColor:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusConstantColor, path, line, column);
	case Keyword::constantAlpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::ConstantAlpha, path, line, column);
	case Keyword::oneMinusConstantAlpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusConstantAlpha, path, line, column);
	case Keyword::srcAlphaSaturate:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::SrcAlphaSaturate, path, line, column);
	case Keyword::src1Color:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::Src1Color, path, line, column);
	case Keyword::oneMinusSrc1Color:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusSrc1Color, path, line, column);
	case Keyword::src1Alpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::Src1Alpha, path, line, column);
	case Keyword::oneMinusSrc1Alpha:
		return TokenData::BlendFactorValue(Grindstone::GraphicsAPI::BlendFactor::OneMinusSrc1Alpha, path, line, column);

	// Blend Operations
	case Keyword::off:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::None, path, line, column);
	case Keyword::add:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Add, path, line, column);
	case Keyword::subtract:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Subtract, path, line, column);
	case Keyword::reverseSubtract:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::ReverseSubtract, path, line, column);
	case Keyword::minimum:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Minimum, path, line, column);
	case Keyword::maximum:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Maximum, path, line, column);
	// case Keyword::zero:
	// 	return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Zero, path, line, column);
	case Keyword::source:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Source, path, line, column);
	case Keyword::destination:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Destination, path, line, column);
	case Keyword::sourceOver:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::SourceOver, path, line, column);
	case Keyword::destinationOver:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::DestinationOver, path, line, column);
	case Keyword::sourceIn:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::SourceIn, path, line, column);
	case Keyword::destinationIn:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::DestinationIn, path, line, column);
	case Keyword::sourceOut:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::SourceOut, path, line, column);
	case Keyword::destinationOut:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::DestinationOut, path, line, column);
	case Keyword::sourceAtop:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::SourceAtop, path, line, column);
	case Keyword::destinationAtop:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::DestinationAtop, path, line, column);
	case Keyword::xOR:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::XOR, path, line, column);
	case Keyword::multiply:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Multiply, path, line, column);
	case Keyword::screen:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Screen, path, line, column);
	case Keyword::overlay:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Overlay, path, line, column);
	case Keyword::darken:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Darken, path, line, column);
	case Keyword::lighten:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Lighten, path, line, column);
	case Keyword::colorDodge:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::ColorDodge, path, line, column);
	case Keyword::colorBurn:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::ColorBurn, path, line, column);
	case Keyword::hardLight:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::HardLight, path, line, column);
	case Keyword::softLight:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::SoftLight, path, line, column);
	case Keyword::difference:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Difference, path, line, column);
	case Keyword::exclusion:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Exclusion, path, line, column);
	case Keyword::invert:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Invert, path, line, column);
	case Keyword::invertRGB:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::InvertRGB, path, line, column);
	case Keyword::linearDodge:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::LinearDodge, path, line, column);
	case Keyword::linearBurn:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::LinearBurn, path, line, column);
	case Keyword::vividLight:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::VividLight, path, line, column);
	case Keyword::linearLight:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::LinearLight, path, line, column);
	case Keyword::pinLight:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::PinLight, path, line, column);
	case Keyword::hardMix:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::HardMix, path, line, column);
	case Keyword::hSLHue:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::HSLHue, path, line, column);
	case Keyword::hSLSaturation:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::HSLSaturation, path, line, column);
	case Keyword::hSLColor:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::HSLColor, path, line, column);
	case Keyword::hSLLuminosity:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::HSLLuminosity, path, line, column);
	case Keyword::plus:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Plus, path, line, column);
	case Keyword::plusClamped:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::PlusClamped, path, line, column);
	case Keyword::plusClampedAlpha:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::PlusClampedAlpha, path, line, column);
	case Keyword::plusDark:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::PlusDark, path, line, column);
	case Keyword::minus:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Minus, path, line, column);
	case Keyword::minusClamped:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::MinusClamped, path, line, column);
	case Keyword::contrast:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Contrast, path, line, column);
	case Keyword::invertOVG:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::InvertOVG, path, line, column);
	case Keyword::red:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Red, path, line, column);
	case Keyword::green:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Green, path, line, column);
	case Keyword::blue:
		return TokenData::BlendOperationValue(Grindstone::GraphicsAPI::BlendOperation::Blue, path, line, column);

	// Shader Stages
	case Keyword::vertex:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::Vertex, path, line, column);
	case Keyword::tesselationEvaluation:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::TesselationEvaluation, path, line, column);
	case Keyword::tesselationControl:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::TesselationControl, path, line, column);
	case Keyword::geometry:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::Geometry, path, line, column);
	case Keyword::fragment:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::Fragment, path, line, column);
	case Keyword::task:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::Task, path, line, column);
	case Keyword::mesh:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::Mesh, path, line, column);
	case Keyword::compute:
		return TokenData::ShaderStageValue(Grindstone::GraphicsAPI::ShaderStage::Compute, path, line, column);
	
	case Keyword::boolTrue:
		return TokenData::Boolean(true, path, line, column);
	case Keyword::boolFalse:
		return TokenData::Boolean(false, path, line, column);
	default:
		std::string errorMsg = std::string("Unhandled token ") + keywordStrings[static_cast<uint8_t>(keyword)];
		scanContext.Log(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Scanner, errorMsg.c_str(), path, line, column);
		return TokenData(Token::Invalid, path, line, column);
	}
}

static TokenData ReadToken(ScanContext& scanContext) {
	SkipWhitespace(scanContext);

	const std::filesystem::path& path = scanContext.path;
	uint32_t line = scanContext.currentLine;
	uint32_t column = scanContext.currentCharacterInLine;

	size_t diff = scanContext.content.end() - scanContext.currentPtr;
	if (diff == 0) {
		return TokenData(Token::EndOfFile, path, line, column);
	}

	char c = *scanContext.currentPtr;

	switch (c) {
	case '[':
		++scanContext.currentPtr;
		return TokenData(Token::OpenSquareBrace, path, line, column);
	case ']':
		++scanContext.currentPtr;
		return TokenData(Token::CloseSquareBrace, path, line, column);
	case '{':
		++scanContext.currentPtr;
		return TokenData(Token::OpenCurly, path, line, column);
	case '}':
		++scanContext.currentPtr;
		return TokenData(Token::CloseCurly, path, line, column);
	case '(':
		++scanContext.currentPtr;
		return TokenData(Token::OpenParenthesis, path, line, column);
	case ')':
		++scanContext.currentPtr;
		return TokenData(Token::CloseParenthesis, path, line, column);
	case ';':
		++scanContext.currentPtr;
		return TokenData(Token::SemiColon, path, line, column);
	case ',':
		++scanContext.currentPtr;
		return TokenData(Token::Comma, path, line, column);
	case ':':
		++scanContext.currentPtr;
		return TokenData(Token::Colon, path, line, column);
	case '"':
		++scanContext.currentPtr;
		return TokenData::String(ReadString(scanContext), path, line, column);
	case '/':
		if (diff >= 2) {
			char nextChar = *(scanContext.currentPtr + 1);
			if (nextChar == '/') {
				return TokenData::Comment(ReadCommentSingleLine(scanContext), path, line, column);
			}
			else if (nextChar == '*') {
				return TokenData::CommentMultiLine(ReadCommentMultiLine(scanContext), path, line, column);
			}
			else {
				return TokenData(Token::Invalid, path, line, column);
			}
		}
	}

	if (c == '.' || (c >= '0' && c <= '9')) {
		float number;
		ReadNumber(scanContext, number); // TODO: Handle error
		return TokenData::Number(number, path, line, column);
	}

	if (std::isalpha(c)) {
		std::string_view identifier = ReadIdentifier(scanContext);
		auto keywordIterator = stringToKeywordMap.find(identifier);
		
		if (keywordIterator != stringToKeywordMap.end()) {
			Keyword keyword = keywordIterator->second;
			return ParseKeywordToken(scanContext, keyword, path, line, column);
		}

		return TokenData::Identifier(identifier, path, line, column);
	}

	++scanContext.currentPtr;
	return TokenData(Token::Invalid, path, line, column);
}

bool ScanPipelineSet(LogCallback logFn, const std::filesystem::path& path, std::string_view content, TokenList& outScannerTokens) {
	ScanContext scanContext(content, path, logFn);

	while (scanContext.currentPtr < scanContext.content.end()) {
		TokenData token = ReadToken(scanContext);

		if (token.token == Token::EndOfFile) {
			return !scanContext.hasError;
		}

		// Ignore comments
		if (token.token == Token::Comment || token.token == Token::CommentMultiLine) {
			continue;
		}

		outScannerTokens.push_back(token);
	}

	return true;
}
