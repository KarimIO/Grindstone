#name Texture Decoder
#renderQueue Decoder
#shaderModule compute
#version 450
#line 6

// Whether to use P2 modes (4 endpoints) for compression. Slow, but improves quality.
#define ENCODE_P2 (QUALITY == 1)

// Improve quality at small performance loss
#define INSET_COLOR_BBOX 1
#define OPTIMIZE_ENDPOINTS 1

// Whether to optimize for luminance error or for RGB error
#define LUMINANCE_WEIGHTS 1

const float HALF_MAX = 65504.0f;
const uint PATTERN_NUM = 32u;

layout(std140, binding = 0) uniform MainCB {
	uvec2 TextureSizeInBlocks;
	vec2 TextureSizeRcp;
} ubo;

layout(binding = 1, rgba32ui) restrict writeonly uniform uimage2D OutputTexture;
layout(binding = 2) uniform sampler2D SrcTexture;

vec3 f32tof16( vec3 value ) {
	return vec3( packHalf2x16( vec2( value.x, 0.0 ) ),
				   packHalf2x16( vec2( value.y, 0.0 ) ),
				   packHalf2x16( vec2( value.z, 0.0 ) ) );
}

vec3 f16tof32( uvec3 value ) {
	return vec3( unpackHalf2x16( value.x ).x,
				   unpackHalf2x16( value.y ).x,
				   unpackHalf2x16( value.z ).x );
}

float f32tof16( float value ) {
	return packHalf2x16( vec2( value.x, 0.0 ) );
}

float f16tof32( uint value ) {
	return unpackHalf2x16( value.x ).x;
}

float CalcMSLE(vec3 a, vec3 b) {
	vec3 delta = log2((b + 1.0f) / (a + 1.0f));
	vec3 deltaSq = delta * delta;

#if LUMINANCE_WEIGHTS
	vec3 luminanceWeights = vec3(0.299f, 0.587f, 0.114f);
	deltaSq *= luminanceWeights;
#endif

	return deltaSq.x + deltaSq.y + deltaSq.z;
}

uint PatternFixupID(uint i) {
	uint ret = 15;
	ret = (((3441033216u >> i) & 0x1u) != 0) ? 2u : ret;
	ret = (((845414400u >> i) & 0x1u) != 0) ? 8u : ret;
	return ret;
}

uint Pattern(uint p, uint i) {
	uint p2 = p / 2u;
	uint p3 = p - p2 * 2u;

	uint enc = 0;
	enc = p2 == 0 ? 2290666700u : enc;
	enc = p2 == 1 ? 3972591342u : enc;
	enc = p2 == 2 ? 4276930688u : enc;
	enc = p2 == 3 ? 3967876808u : enc;
	enc = p2 == 4 ? 4293707776u : enc;
	enc = p2 == 5 ? 3892379264u : enc;
	enc = p2 == 6 ? 4278255592u : enc;
	enc = p2 == 7 ? 4026597360u : enc;
	enc = p2 == 8 ? 9369360u : enc;
	enc = p2 == 9 ? 147747072u : enc;
	enc = p2 == 10 ? 1930428556u : enc;
	enc = p2 == 11 ? 2362323200u : enc;
	enc = p2 == 12 ? 823134348u : enc;
	enc = p2 == 13 ? 913073766u : enc;
	enc = p2 == 14 ? 267393000u : enc;
	enc = p2 == 15 ? 966553998u : enc;

	enc = (p3 != 0u) ? enc >> 16u : enc;
	uint ret = (enc >> i) & 0x1u;
	return ret;
}

vec3 Quantize7(vec3 x) {
	return (f32tof16(x) * 128.0f) / (0x7bff + 1.0f);
}

vec3 Quantize9(vec3 x) {
	return (f32tof16(x) * 512.0f) / (0x7bff + 1.0f);
}

vec3 Quantize10(vec3 x) {
	return (f32tof16(x) * 1024.0f) / (0x7bff + 1.0f);
}

vec3 Unquantize7(vec3 x) {
	return (x * 65536.0f + 0x8000) / 128.0f;
}

vec3 Unquantize9(vec3 x) {
	return (x * 65536.0f + 0x8000) / 512.0f;
}

vec3 Unquantize10(vec3 x) {
	return (x * 65536.0f + 0x8000) / 1024.0f;
}

vec3 FinishUnquantize(vec3 endpoint0Unq, vec3 endpoint1Unq, float weight) {
	vec3 comp = (endpoint0Unq * (64.0f - weight) + endpoint1Unq * weight + 32.0f) * (31.0f / 4096.0f);
	return f16tof32(uvec3(comp));
}

void Swap(inout vec3 a, inout vec3 b) {
	vec3 tmp = a;
	a = b;
	b = tmp;
}

void Swap(inout float a, inout float b) {
	float tmp = a;
	a = b;
	b = tmp;
}

uint ComputeIndex3(float texelPos, float endPoint0Pos, float endPoint1Pos) {
	float r = (texelPos - endPoint0Pos) / (endPoint1Pos - endPoint0Pos);
	return uint(clamp(r * 6.98182f + 0.00909f + 0.5f, 0.0f, 7.0f));
}

uint ComputeIndex4(float texelPos, float endPoint0Pos, float endPoint1Pos) {
	float r = (texelPos - endPoint0Pos) / (endPoint1Pos - endPoint0Pos);
	return uint(clamp(r * 14.93333f + 0.03333f + 0.5f, 0.0f, 15.0f));
}

void SignExtend(inout vec3 v1, uint mask, uint signFlag) {
	ivec3 v = ivec3(v1);
	v.x = (v.x & int(mask)) | (v.x < 0 ? int(signFlag) : 0);
	v.y = (v.y & int(mask)) | (v.y < 0 ? int(signFlag) : 0);
	v.z = (v.z & int(mask)) | (v.z < 0 ? int(signFlag) : 0);
	v1 = v;
}

// Refine endpoints by insetting bounding box in log2 RGB space
void InsetColorBBoxP1(vec3 texels[16], inout vec3 blockMin, inout vec3 blockMax) {
	vec3 refinedBlockMin = blockMax;
	vec3 refinedBlockMax = blockMin;

	for (uint i = 0; i < 16u; ++i) {
		refinedBlockMin = min(refinedBlockMin, texels[i] == blockMin ? refinedBlockMin : texels[i]);
		refinedBlockMax = max(refinedBlockMax, texels[i] == blockMax ? refinedBlockMax : texels[i]);
	}

	vec3 logRefinedBlockMax = log2(refinedBlockMax + 1.0f);
	vec3 logRefinedBlockMin = log2(refinedBlockMin + 1.0f);

	vec3 logBlockMax = log2(blockMax + 1.0f);
	vec3 logBlockMin = log2(blockMin + 1.0f);
	vec3 logBlockMaxExt = (logBlockMax - logBlockMin) * (1.0f / 32.0f);

	logBlockMin += min(logRefinedBlockMin - logBlockMin, logBlockMaxExt);
	logBlockMax -= min(logBlockMax - logRefinedBlockMax, logBlockMaxExt);

	blockMin = exp2(logBlockMin) - 1.0f;
	blockMax = exp2(logBlockMax) - 1.0f;
}

// Refine endpoints by insetting bounding box in log2 RGB space
void InsetColorBBoxP2(vec3 texels[16], uint pattern, uint patternSelector, inout vec3 blockMin, inout vec3 blockMax) {
	vec3 refinedBlockMin = blockMax;
	vec3 refinedBlockMax = blockMin;

	for (uint i = 0; i < 16; ++i) {
		uint paletteID = Pattern(pattern, i);
		if (paletteID == patternSelector) {
			refinedBlockMin = min(refinedBlockMin, texels[i] == blockMin ? refinedBlockMin : texels[i]);
			refinedBlockMax = max(refinedBlockMax, texels[i] == blockMax ? refinedBlockMax : texels[i]);
		}
	}

	vec3 logRefinedBlockMax = log2(refinedBlockMax + 1.0f);
	vec3 logRefinedBlockMin = log2(refinedBlockMin + 1.0f);

	vec3 logBlockMax = log2(blockMax + 1.0f);
	vec3 logBlockMin = log2(blockMin + 1.0f);
	vec3 logBlockMaxExt = (logBlockMax - logBlockMin) * (1.0f / 32.0f);

	logBlockMin += min(logRefinedBlockMin - logBlockMin, logBlockMaxExt);
	logBlockMax -= min(logBlockMax - logRefinedBlockMax, logBlockMaxExt);

	blockMin = exp2(logBlockMin) - 1.0f;
	blockMax = exp2(logBlockMax) - 1.0f;
}

// Least squares optimization to find best endpoints for the selected block indices
void OptimizeEndpointsP1(vec3 texels[16], inout vec3 blockMin, inout vec3 blockMax, in vec3 blockMinNonInset, in vec3 blockMaxNonInset) {
	vec3 blockDir = blockMax - blockMin;
	blockDir = blockDir / (blockDir.x + blockDir.y + blockDir.z);

	float endPoint0Pos = f32tof16(dot(blockMin, blockDir));
	float endPoint1Pos = f32tof16(dot(blockMax, blockDir));

	vec3 alphaTexelSum = vec3(0.0f);
	vec3 betaTexelSum = vec3(0.0f);
	float alphaBetaSum = 0.0f;
	float alphaSqSum = 0.0f;
	float betaSqSum = 0.0f;

	for (int i = 0; i < 16; i++) {
		float texelPos = f32tof16(dot(texels[i], blockDir));
		uint texelIndex = ComputeIndex4(texelPos, endPoint0Pos, endPoint1Pos);

		float beta = clamp(texelIndex / 15.0f, 0.0f, 1.0f);
		float alpha = 1.0f - beta;

		vec3 texelF16 = f32tof16(texels[i].xyz);
		alphaTexelSum += alpha * texelF16;
		betaTexelSum += beta * texelF16;

		alphaBetaSum += alpha * beta;

		alphaSqSum += alpha * alpha;
		betaSqSum += beta * beta;
	}

	float det = alphaSqSum * betaSqSum - alphaBetaSum * alphaBetaSum;

	if (abs(det) > 0.00001f) {
		float detRcp = 1.0f / det;
		blockMin = clamp(f16tof32(uvec3(clamp(detRcp * (alphaTexelSum * betaSqSum - betaTexelSum * alphaBetaSum), 0.0f, HALF_MAX))), blockMinNonInset, blockMaxNonInset);
		blockMax = clamp(f16tof32(uvec3(clamp(detRcp * (betaTexelSum * alphaSqSum - alphaTexelSum * alphaBetaSum), 0.0f, HALF_MAX))), blockMinNonInset, blockMaxNonInset);
	}
}

// Least squares optimization to find best endpoints for the selected block indices
void OptimizeEndpointsP2(vec3 texels[16], uint pattern, uint patternSelector, inout vec3 blockMin, inout vec3 blockMax) {
	vec3 blockDir = blockMax - blockMin;
	blockDir = blockDir / (blockDir.x + blockDir.y + blockDir.z);

	float endPoint0Pos = f32tof16(dot(blockMin, blockDir));
	float endPoint1Pos = f32tof16(dot(blockMax, blockDir));

	vec3 alphaTexelSum = vec3(0.0f);
	vec3 betaTexelSum = vec3(0.0f);
	float alphaBetaSum = 0.0f;
	float alphaSqSum = 0.0f;
	float betaSqSum = 0.0f;

	for (int i = 0; i < 16; i++) {
		uint paletteID = Pattern(pattern, i);
		if (paletteID == patternSelector) {
			float texelPos = f32tof16(dot(texels[i], blockDir));
			uint texelIndex = ComputeIndex3(texelPos, endPoint0Pos, endPoint1Pos);

			float beta = clamp(texelIndex / 7.0f, 0.0f, 1.0f);
			float alpha = 1.0f - beta;

			vec3 texelF16 = f32tof16(texels[i].xyz);
			alphaTexelSum += alpha * texelF16;
			betaTexelSum += beta * texelF16;

			alphaBetaSum += alpha * beta;

			alphaSqSum += alpha * alpha;
			betaSqSum += beta * beta;
		}
	}

	float det = alphaSqSum * betaSqSum - alphaBetaSum * alphaBetaSum;

	if (abs(det) > 0.00001f) {
		float detRcp = 1.0f / det;
		blockMin = f16tof32(uvec3(clamp(detRcp * (alphaTexelSum * betaSqSum - betaTexelSum * alphaBetaSum), 0.0f, HALF_MAX)));
		blockMax = f16tof32(uvec3(clamp(detRcp * (betaTexelSum * alphaSqSum - alphaTexelSum * alphaBetaSum), 0.0f, HALF_MAX)));
	}
}

void EncodeP1(inout uvec4 block, inout float blockMSLE, vec3 texels[16]) {
	// compute endpoints (min/max RGB bbox)
	vec3 blockMin = texels[0];
	vec3 blockMax = texels[0];
	for (uint i = 1; i < 16; ++i) {
		blockMin = min(blockMin, texels[i]);
		blockMax = max(blockMax, texels[i]);
	}

	vec3 blockMinNonInset = blockMin;
	vec3 blockMaxNonInset = blockMax;
#if INSET_COLOR_BBOX
	InsetColorBBoxP1(texels, blockMin, blockMax);
#endif

#if OPTIMIZE_ENDPOINTS
	OptimizeEndpointsP1(texels, blockMin, blockMax, blockMinNonInset, blockMaxNonInset);
#endif


	vec3 blockDir = blockMax - blockMin;
	blockDir = blockDir / (blockDir.x + blockDir.y + blockDir.z);

	vec3 endpoint0 = Quantize10(blockMin);
	vec3 endpoint1 = Quantize10(blockMax);
	float endPoint0Pos = f32tof16(dot(blockMin, blockDir));
	float endPoint1Pos = f32tof16(dot(blockMax, blockDir));

	// check if endpoint swap is required
	float fixupTexelPos = f32tof16(dot(texels[0], blockDir));
	uint fixupIndex = ComputeIndex4(fixupTexelPos, endPoint0Pos, endPoint1Pos);
	if (fixupIndex > 7) {
		Swap(endPoint0Pos, endPoint1Pos);
		Swap(endpoint0, endpoint1);
	}

	// compute indices
	uint indices[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	for (uint i = 0; i < 16; ++i) {
		float texelPos = f32tof16(dot(texels[i], blockDir));
		indices[i] = ComputeIndex4(texelPos, endPoint0Pos, endPoint1Pos);
	}

	// compute compression error (MSLE)
	vec3 endpoint0Unq = Unquantize10(endpoint0);
	vec3 endpoint1Unq = Unquantize10(endpoint1);
	float msle = 0.0f;
	for (uint i = 0; i < 16; ++i) {
		float weight = floor((indices[i] * 64.0f) / 15.0f + 0.5f);
		vec3 texelUnc = FinishUnquantize(endpoint0Unq, endpoint1Unq, weight);

		msle += CalcMSLE(texels[i], texelUnc);
	}


	// encode block for mode 11
	blockMSLE = msle;
	block.x = 0x03;

	// endpoints
	block.x |= uint(endpoint0.x) << 5u;
	block.x |= uint(endpoint0.y) << 15u;
	block.x |= uint(endpoint0.z) << 25u;
	block.y |= uint(endpoint0.z) >> 7u;
	block.y |= uint(endpoint1.x) << 3u;
	block.y |= uint(endpoint1.y) << 13u;
	block.y |= uint(endpoint1.z) << 23u;
	block.z |= uint(endpoint1.z) >> 9u;

	// indices
	block.z |= indices[0] << 1u;
	block.z |= indices[1] << 4u;
	block.z |= indices[2] << 8u;
	block.z |= indices[3] << 12u;
	block.z |= indices[4] << 16u;
	block.z |= indices[5] << 20u;
	block.z |= indices[6] << 24u;
	block.z |= indices[7] << 28u;
	block.w |= indices[8] << 0u;
	block.w |= indices[9] << 4u;
	block.w |= indices[10] << 8u;
	block.w |= indices[11] << 12u;
	block.w |= indices[12] << 16u;
	block.w |= indices[13] << 20u;
	block.w |= indices[14] << 24u;
	block.w |= indices[15] << 28u;
}

float DistToLineSq(vec3 PointOnLine, vec3 LineDirection, vec3 Point) {
	vec3 w = Point - PointOnLine;
	vec3 x = w - dot(w, LineDirection) * LineDirection;
	return dot(x, x);
}

// Evaluate how good is given P2 pattern for encoding current block
float EvaluateP2Pattern(int pattern, vec3 texels[16]) {
	vec3 p0BlockMin = vec3(HALF_MAX, HALF_MAX, HALF_MAX);
	vec3 p0BlockMax = vec3(0.0f, 0.0f, 0.0f);
	vec3 p1BlockMin = vec3(HALF_MAX, HALF_MAX, HALF_MAX);
	vec3 p1BlockMax = vec3(0.0f, 0.0f, 0.0f);

	for (uint i = 0; i < 16; ++i) {
		uint paletteID = Pattern(pattern, i);
		if (paletteID == 0) {
			p0BlockMin = min(p0BlockMin, texels[i]);
			p0BlockMax = max(p0BlockMax, texels[i]);
		}
		else
		{
			p1BlockMin = min(p1BlockMin, texels[i]);
			p1BlockMax = max(p1BlockMax, texels[i]);
		}
	}

	vec3 p0BlockDir = normalize(p0BlockMax - p0BlockMin);
	vec3 p1BlockDir = normalize(p1BlockMax - p1BlockMin);

	float sqDistanceFromLine = 0.0f;

	for (uint i = 0; i < 16; ++i) {
		uint paletteID = Pattern(pattern, i);
		if (paletteID == 0) {
			sqDistanceFromLine += DistToLineSq(p0BlockMin, p0BlockDir, texels[i]);
		}
		else
		{
			sqDistanceFromLine += DistToLineSq(p1BlockMin, p1BlockDir, texels[i]);
		}
	}

	return sqDistanceFromLine;
}

void EncodeP2Pattern(inout uvec4 block, inout float blockMSLE, int pattern, vec3 texels[16]) {
	vec3 p0BlockMin = vec3(HALF_MAX, HALF_MAX, HALF_MAX);
	vec3 p0BlockMax = vec3(0.0f, 0.0f, 0.0f);
	vec3 p1BlockMin = vec3(HALF_MAX, HALF_MAX, HALF_MAX);
	vec3 p1BlockMax = vec3(0.0f, 0.0f, 0.0f);

	for (uint i = 0; i < 16; ++i) {
		uint paletteID = Pattern(pattern, i);
		if (paletteID == 0) {
			p0BlockMin = min(p0BlockMin, texels[i]);
			p0BlockMax = max(p0BlockMax, texels[i]);
		}
		else
		{
			p1BlockMin = min(p1BlockMin, texels[i]);
			p1BlockMax = max(p1BlockMax, texels[i]);
		}
	}

#if INSET_COLOR_BBOX
	// Disabled because it was a negligible quality increase
	//InsetColorBBoxP2(texels, pattern, 0, p0BlockMin, p0BlockMax);
	//InsetColorBBoxP2(texels, pattern, 1, p1BlockMin, p1BlockMax);
#endif

#if OPTIMIZE_ENDPOINTS
	OptimizeEndpointsP2(texels, pattern, 0, p0BlockMin, p0BlockMax);
	OptimizeEndpointsP2(texels, pattern, 1, p1BlockMin, p1BlockMax);
#endif

	vec3 p0BlockDir = p0BlockMax - p0BlockMin;
	vec3 p1BlockDir = p1BlockMax - p1BlockMin;
	p0BlockDir = p0BlockDir / (p0BlockDir.x + p0BlockDir.y + p0BlockDir.z);
	p1BlockDir = p1BlockDir / (p1BlockDir.x + p1BlockDir.y + p1BlockDir.z);


	float p0Endpoint0Pos = f32tof16(dot(p0BlockMin, p0BlockDir));
	float p0Endpoint1Pos = f32tof16(dot(p0BlockMax, p0BlockDir));
	float p1Endpoint0Pos = f32tof16(dot(p1BlockMin, p1BlockDir));
	float p1Endpoint1Pos = f32tof16(dot(p1BlockMax, p1BlockDir));


	uint fixupID = PatternFixupID(pattern);
	float p0FixupTexelPos = f32tof16(dot(texels[0], p0BlockDir));
	float p1FixupTexelPos = f32tof16(dot(texels[fixupID], p1BlockDir));
	uint p0FixupIndex = ComputeIndex3(p0FixupTexelPos, p0Endpoint0Pos, p0Endpoint1Pos);
	uint p1FixupIndex = ComputeIndex3(p1FixupTexelPos, p1Endpoint0Pos, p1Endpoint1Pos);
	if (p0FixupIndex > 3) {
		Swap(p0Endpoint0Pos, p0Endpoint1Pos);
		Swap(p0BlockMin, p0BlockMax);
	}
	if (p1FixupIndex > 3) {
		Swap(p1Endpoint0Pos, p1Endpoint1Pos);
		Swap(p1BlockMin, p1BlockMax);
	}

	uint indices[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	for (uint i = 0; i < 16; ++i) {
		float p0TexelPos = f32tof16(dot(texels[i], p0BlockDir));
		float p1TexelPos = f32tof16(dot(texels[i], p1BlockDir));
		uint p0Index = ComputeIndex3(p0TexelPos, p0Endpoint0Pos, p0Endpoint1Pos);
		uint p1Index = ComputeIndex3(p1TexelPos, p1Endpoint0Pos, p1Endpoint1Pos);

		uint paletteID = Pattern(pattern, i);
		indices[i] = paletteID == 0 ? p0Index : p1Index;
	}

	vec3 endpoint760 = floor(Quantize7(p0BlockMin));
	vec3 endpoint761 = floor(Quantize7(p0BlockMax));
	vec3 endpoint762 = floor(Quantize7(p1BlockMin));
	vec3 endpoint763 = floor(Quantize7(p1BlockMax));

	vec3 endpoint950 = floor(Quantize9(p0BlockMin));
	vec3 endpoint951 = floor(Quantize9(p0BlockMax));
	vec3 endpoint952 = floor(Quantize9(p1BlockMin));
	vec3 endpoint953 = floor(Quantize9(p1BlockMax));

	endpoint761 = endpoint761 - endpoint760;
	endpoint762 = endpoint762 - endpoint760;
	endpoint763 = endpoint763 - endpoint760;

	endpoint951 = endpoint951 - endpoint950;
	endpoint952 = endpoint952 - endpoint950;
	endpoint953 = endpoint953 - endpoint950;

	int maxVal76 = 0x1F;
	endpoint761 = clamp(endpoint761, -maxVal76, maxVal76);
	endpoint762 = clamp(endpoint762, -maxVal76, maxVal76);
	endpoint763 = clamp(endpoint763, -maxVal76, maxVal76);

	int maxVal95 = 0xF;
	endpoint951 = clamp(endpoint951, -maxVal95, maxVal95);
	endpoint952 = clamp(endpoint952, -maxVal95, maxVal95);
	endpoint953 = clamp(endpoint953, -maxVal95, maxVal95);

	vec3 endpoint760Unq = Unquantize7(endpoint760);
	vec3 endpoint761Unq = Unquantize7(endpoint760 + endpoint761);
	vec3 endpoint762Unq = Unquantize7(endpoint760 + endpoint762);
	vec3 endpoint763Unq = Unquantize7(endpoint760 + endpoint763);
	vec3 endpoint950Unq = Unquantize9(endpoint950);
	vec3 endpoint951Unq = Unquantize9(endpoint950 + endpoint951);
	vec3 endpoint952Unq = Unquantize9(endpoint950 + endpoint952);
	vec3 endpoint953Unq = Unquantize9(endpoint950 + endpoint953);

	float msle76 = 0.0f;
	float msle95 = 0.0f;
	for (uint i = 0; i < 16; ++i) {
		uint paletteID = Pattern(pattern, i);

		vec3 tmp760Unq = paletteID == 0 ? endpoint760Unq : endpoint762Unq;
		vec3 tmp761Unq = paletteID == 0 ? endpoint761Unq : endpoint763Unq;
		vec3 tmp950Unq = paletteID == 0 ? endpoint950Unq : endpoint952Unq;
		vec3 tmp951Unq = paletteID == 0 ? endpoint951Unq : endpoint953Unq;

		float weight = floor((indices[i] * 64.0f) / 7.0f + 0.5f);
		vec3 texelUnc76 = FinishUnquantize(tmp760Unq, tmp761Unq, weight);
		vec3 texelUnc95 = FinishUnquantize(tmp950Unq, tmp951Unq, weight);

		msle76 += CalcMSLE(texels[i], texelUnc76);
		msle95 += CalcMSLE(texels[i], texelUnc95);
	}

	SignExtend(endpoint761, 0x1F, 0x20);
	SignExtend(endpoint762, 0x1F, 0x20);
	SignExtend(endpoint763, 0x1F, 0x20);

	SignExtend(endpoint951, 0xF, 0x10);
	SignExtend(endpoint952, 0xF, 0x10);
	SignExtend(endpoint953, 0xF, 0x10);

	// encode block
	float p2MSLE = min(msle76, msle95);
	if (p2MSLE < blockMSLE) {
		blockMSLE = p2MSLE;
		block = uvec4(0, 0, 0, 0);

		if (p2MSLE == msle76) {
			// 7.6
			block.x = 0x1u;
			block.x |= ( uint( endpoint762.y ) & 0x20u ) >> 3u;
			block.x |= ( uint( endpoint763.y ) & 0x10u ) >> 1u;
			block.x |= ( uint( endpoint763.y ) & 0x20u ) >> 1u;
			block.x |= uint( endpoint760.x ) << 5u;
			block.x |= ( uint( endpoint763.z ) & 0x01u ) << 12u;
			block.x |= ( uint( endpoint763.z ) & 0x02u ) << 12u;
			block.x |= ( uint( endpoint762.z ) & 0x10u ) << 10u;
			block.x |= uint( endpoint760.y ) << 15u;
			block.x |= ( uint( endpoint762.z ) & 0x20u ) << 17u;
			block.x |= ( uint( endpoint763.z ) & 0x04u ) << 21u;
			block.x |= ( uint( endpoint762.y ) & 0x10u ) << 20u;
			block.x |= uint( endpoint760.z ) << 25u;
			block.y |= ( uint( endpoint763.z ) & 0x08u ) >> 3u;
			block.y |= ( uint( endpoint763.z ) & 0x20u ) >> 4u;
			block.y |= ( uint( endpoint763.z ) & 0x10u ) >> 2u;
			block.y |= uint( endpoint761.x ) << 3u;
			block.y |= ( uint( endpoint762.y ) & 0x0Fu ) << 9u;
			block.y |= uint( endpoint761.y ) << 13u;
			block.y |= ( uint( endpoint763.y ) & 0x0Fu ) << 19u;
			block.y |= uint( endpoint761.z ) << 23u;
			block.y |= ( uint( endpoint762.z ) & 0x07u ) << 29u;
			block.z |= ( uint( endpoint762.z ) & 0x08u ) >> 3u;
			block.z |= uint( endpoint762.x ) << 1u;
			block.z |= uint( endpoint763.x ) << 7u;
		}
		else {
			// 9.5
			block.x = 0xEu;
			block.x |= uint( endpoint950.x ) << 5u;
			block.x |= ( uint( endpoint952.z ) & 0x10u ) << 10u;
			block.x |= uint( endpoint950.y ) << 15u;
			block.x |= ( uint( endpoint952.y ) & 0x10u ) << 20u;
			block.x |= uint( endpoint950.z ) << 25u;
			block.y |= uint( endpoint950.z ) >> 7u;
			block.y |= ( uint( endpoint953.z ) & 0x10u ) >> 2u;
			block.y |= uint( endpoint951.x ) << 3u;
			block.y |= ( uint( endpoint953.y ) & 0x10u ) << 4u;
			block.y |= ( uint( endpoint952.y ) & 0x0Fu ) << 9u;
			block.y |= uint( endpoint951.y ) << 13u;
			block.y |= ( uint( endpoint953.z ) & 0x01u ) << 18u;
			block.y |= ( uint( endpoint953.y ) & 0x0Fu ) << 19u;
			block.y |= uint( endpoint951.z ) << 23u;
			block.y |= ( uint( endpoint953.z ) & 0x02u ) << 27u;
			block.y |= uint( endpoint952.z ) << 29u;
			block.z |= ( uint( endpoint952.z ) & 0x08u ) >> 3u;
			block.z |= uint( endpoint952.x ) << 1u;
			block.z |= ( uint( endpoint953.z ) & 0x04u ) << 4u;
			block.z |= uint( endpoint953.x ) << 7u;
			block.z |= ( uint( endpoint953.z ) & 0x08u ) << 9u;
		}

		block.z |= pattern << 13;
		uint blockFixupID = PatternFixupID(pattern);
		if (blockFixupID == 15) {
			block.z |= indices[0] << 18u;
			block.z |= indices[1] << 20u;
			block.z |= indices[2] << 23u;
			block.z |= indices[3] << 26u;
			block.z |= indices[4] << 29u;
			block.w |= indices[5] << 0u;
			block.w |= indices[6] << 3u;
			block.w |= indices[7] << 6u;
			block.w |= indices[8] << 9u;
			block.w |= indices[9] << 12u;
			block.w |= indices[10] << 15u;
			block.w |= indices[11] << 18u;
			block.w |= indices[12] << 21u;
			block.w |= indices[13] << 24u;
			block.w |= indices[14] << 27u;
			block.w |= indices[15] << 30u;
		}
		else if (blockFixupID == 2) {
			block.z |= indices[0] << 18u;
			block.z |= indices[1] << 20u;
			block.z |= indices[2] << 23u;
			block.z |= indices[3] << 25u;
			block.z |= indices[4] << 28u;
			block.z |= indices[5] << 31u;
			block.w |= indices[5] >> 1u;
			block.w |= indices[6] << 2u;
			block.w |= indices[7] << 5u;
			block.w |= indices[8] << 8u;
			block.w |= indices[9] << 11u;
			block.w |= indices[10] << 14u;
			block.w |= indices[11] << 17u;
			block.w |= indices[12] << 20u;
			block.w |= indices[13] << 23u;
			block.w |= indices[14] << 26u;
			block.w |= indices[15] << 29u;
		}
		else
		{
			block.z |= indices[0] << 18u;
			block.z |= indices[1] << 20u;
			block.z |= indices[2] << 23u;
			block.z |= indices[3] << 26u;
			block.z |= indices[4] << 29u;
			block.w |= indices[5] << 0u;
			block.w |= indices[6] << 3u;
			block.w |= indices[7] << 6u;
			block.w |= indices[8] << 9u;
			block.w |= indices[9] << 11u;
			block.w |= indices[10] << 14u;
			block.w |= indices[11] << 17u;
			block.w |= indices[12] << 20u;
			block.w |= indices[13] << 23u;
			block.w |= indices[14] << 26u;
			block.w |= indices[15] << 29u;
		}
	}
}

#define GatherRed( tex, uv ) textureGather( tex, uv, 0 )
#define GatherGreen( tex, uv ) textureGather( tex, uv, 1 )
#define GatherBlue( tex, uv ) textureGather( tex, uv, 2 )

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main() {
	// Gather texels for current 4x4 block
	// 0 1 2 3
	// 4 5 6 7
	// 8 9 10 11
	// 12 13 14 15
	vec2 uv = gl_GlobalInvocationID.xy * ubo.TextureSizeRcp * 4.0f + ubo.TextureSizeRcp;
	vec2 block0UV = uv;
	vec2 block1UV = uv + vec2(2.0f * ubo.TextureSizeRcp.x, 0.0f);
	vec2 block2UV = uv + vec2(0.0f, 2.0f * ubo.TextureSizeRcp.y);
	vec2 block3UV = uv + vec2(2.0f * ubo.TextureSizeRcp.x, 2.0f * ubo.TextureSizeRcp.y);
	vec4 block0X = GatherRed(SrcTexture, block0UV);
	vec4 block1X = GatherRed(SrcTexture, block1UV);
	vec4 block2X = GatherRed(SrcTexture, block2UV);
	vec4 block3X = GatherRed(SrcTexture, block3UV);
	vec4 block0Y = GatherGreen(SrcTexture, block0UV);
	vec4 block1Y = GatherGreen(SrcTexture, block1UV);
	vec4 block2Y = GatherGreen(SrcTexture, block2UV);
	vec4 block3Y = GatherGreen(SrcTexture, block3UV);
	vec4 block0Z = GatherBlue(SrcTexture, block0UV);
	vec4 block1Z = GatherBlue(SrcTexture, block1UV);
	vec4 block2Z = GatherBlue(SrcTexture, block2UV);
	vec4 block3Z = GatherBlue(SrcTexture, block3UV);

	vec3 texels[16];
	texels[0] = vec3(block0X.w, block0Y.w, block0Z.w);
	texels[1] = vec3(block0X.z, block0Y.z, block0Z.z);
	texels[2] = vec3(block1X.w, block1Y.w, block1Z.w);
	texels[3] = vec3(block1X.z, block1Y.z, block1Z.z);
	texels[4] = vec3(block0X.x, block0Y.x, block0Z.x);
	texels[5] = vec3(block0X.y, block0Y.y, block0Z.y);
	texels[6] = vec3(block1X.x, block1Y.x, block1Z.x);
	texels[7] = vec3(block1X.y, block1Y.y, block1Z.y);
	texels[8] = vec3(block2X.w, block2Y.w, block2Z.w);
	texels[9] = vec3(block2X.z, block2Y.z, block2Z.z);
	texels[10] = vec3(block3X.w, block3Y.w, block3Z.w);
	texels[11] = vec3(block3X.z, block3Y.z, block3Z.z);
	texels[12] = vec3(block2X.x, block2Y.x, block2Z.x);
	texels[13] = vec3(block2X.y, block2Y.y, block2Z.y);
	texels[14] = vec3(block3X.x, block3Y.x, block3Z.x);
	texels[15] = vec3(block3X.y, block3Y.y, block3Z.y);

	uvec4 block = uvec4(0, 0, 0, 0);
	float blockMSLE = 0.0f;

	EncodeP1(block, blockMSLE, texels);

#if ENCODE_P2
	// First find pattern which is a best fit for a current block
	float bestScore = EvaluateP2Pattern(0, texels);
	uint bestPattern = 0;

	for (uint patternIndex = 1; patternIndex < 32; ++patternIndex) {
		float score = EvaluateP2Pattern(patternIndex, texels);
		if (score < bestScore) {
			bestPattern = patternIndex;
			bestScore = score;
		}
	}

	// Then encode it
	EncodeP2Pattern(block, blockMSLE, bestPattern, texels);
#endif

	imageStore( OutputTexture, ivec2( gl_GlobalInvocationID.xy ), block );
}
#endShaderModule
