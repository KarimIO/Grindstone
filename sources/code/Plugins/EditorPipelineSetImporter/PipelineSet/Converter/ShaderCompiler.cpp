#include <fmt/format.h>
#include <vector>
#include "ResolvedStateTree.hpp"
#include "ShaderCompiler.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <spirv_reflect.h>
#include <Common/Assert.hpp>

static void PrintShaderMessage(LogCallback logCallback, const std::filesystem::path& path, const std::string& msg) {
	Grindstone::LogSeverity severity = Grindstone::LogSeverity::Info;

	if (msg.find("error") != std::string::npos) {
		severity = Grindstone::LogSeverity::Error;
	}

	// TODO: Unsupress warnings.
	if (msg.find("warning") != std::string::npos) {
		severity = Grindstone::LogSeverity::Warning;
	}
	else {
		logCallback(severity, PipelineConverterLogSource::Output, msg, path, UNDEFINED_LINE, UNDEFINED_COLUMN);
	}
}

static int Filter(unsigned int code, struct _EXCEPTION_POINTERS* pExceptionInfo) {
	static char scratch[32];
	// report all errors with fputs to prevent any allocation
	if (code == EXCEPTION_ACCESS_VIOLATION) {
		// use pExceptionInfo to document and report error
		fputs("access violation. Attempted to ", stderr);
		if (pExceptionInfo->ExceptionRecord->ExceptionInformation[0])
			fputs("write", stderr);
		else
			fputs("read", stderr);
		fputs(" from address ", stderr);
		sprintf_s(scratch, _countof(scratch), "0x%p\n",
			(void*)pExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
		fputs(scratch, stderr);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	if (code == EXCEPTION_STACK_OVERFLOW) {
		// use pExceptionInfo to document and report error
		fputs("stack overflow\n", stderr);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	fputs("Unrecoverable Error ", stderr);
	sprintf_s(scratch, _countof(scratch), "0x%08x\n", code);
	fputs(scratch, stderr);
	return EXCEPTION_CONTINUE_SEARCH;
}

static HRESULT Compile(IDxcCompiler3* pCompiler, DxcBuffer* pSource, LPCWSTR pszArgs[],
	int argCt, IDxcIncludeHandler* pIncludeHandler, IDxcResult** compiledShaderBuffer) {

	__try {
		return pCompiler->Compile(
			pSource,                // Source buffer.
			pszArgs,                // Array of pointers to arguments.
			argCt,                  // Number of arguments.
			pIncludeHandler,        // User-provided interface to handle #include directives (optional).
			IID_PPV_ARGS(compiledShaderBuffer) // Compiler output status, buffer, and errors.
		);
	}
	__except (Filter(GetExceptionCode(), GetExceptionInformation())) {
		// UNRECOVERABLE ERROR!
		// At this point, state could be extremely corrupt. Terminate the process
		return E_FAIL;
	}
}

static bool GatherArtifactsDirectX(IDxcUtils* pUtils, IDxcResult* pResults, StageCompilationArtifacts& outArtifacts) {
	Microsoft::WRL::ComPtr<IDxcBlob> pShader = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
	if (SUCCEEDED(pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName)) &&
		pShader != nullptr
		) {
		outArtifacts.compiledCode.resize(pShader->GetBufferSize());
		memcpy(outArtifacts.compiledCode.data(), pShader->GetBufferPointer(), pShader->GetBufferSize());
	}

	//
	// Save pdb.
	//
	Microsoft::WRL::ComPtr<IDxcBlob> pPDB = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
	if (SUCCEEDED(pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName))) {
		outArtifacts.compiledPdb.resize(pPDB->GetBufferSize());
		memcpy(outArtifacts.compiledPdb.data(), pPDB->GetBufferPointer(), pPDB->GetBufferSize());
	}

	//
	// Print hash.
	//
	Microsoft::WRL::ComPtr<IDxcBlob> pHash = nullptr;
	if (SUCCEEDED(pResults->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHash), nullptr)) &&
		pHash != nullptr) {
		outArtifacts.hash.resize(pHash->GetBufferSize());
		memcpy(outArtifacts.hash.data(), pHash->GetBufferPointer(), pHash->GetBufferSize());
	}


	//
	// Get separate reflection.
	//
	Microsoft::WRL::ComPtr<IDxcBlob> pReflectionData;
	if (SUCCEEDED(pResults->GetOutput(DXC_OUT_HLSL, IID_PPV_ARGS(&pReflectionData), nullptr)) &&
		pReflectionData != nullptr) {
		DxcBuffer reflectionBuffer{};
		reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
		reflectionBuffer.Size = pReflectionData->GetBufferSize();
		reflectionBuffer.Encoding = 0;

		Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection{};
		pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
		D3D12_SHADER_DESC shaderDesc{};
		shaderReflection->GetDesc(&shaderDesc);
	}

	return true;
}

static bool GatherArtifactsSpirV(IDxcUtils* pUtils, IDxcResult* pResults, StageCompilationArtifacts& outArtifacts) {
	Microsoft::WRL::ComPtr<IDxcBlob> pShader = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
	if (FAILED(pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName)) || pShader == nullptr) {
		GS_ASSERT_LOG("Failed to compile shader.");
		return false;
	}

	outArtifacts.compiledCode.resize(pShader->GetBufferSize());
	memcpy(outArtifacts.compiledCode.data(), pShader->GetBufferPointer(), pShader->GetBufferSize());

	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(outArtifacts.compiledCode.size(), outArtifacts.compiledCode.data(), &module);
	GS_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

	uint32_t descriptorInputCount = 0;
	result = spvReflectEnumerateDescriptorSets(&module, &descriptorInputCount, nullptr);
	GS_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
	std::vector<SpvReflectDescriptorSet*> descriptorSets;
	descriptorSets.resize(descriptorInputCount);
	result = spvReflectEnumerateDescriptorSets(&module, &descriptorInputCount, descriptorSets.data());
	GS_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

	for (uint32_t descriptorSetIndex = 0; descriptorSetIndex < descriptorInputCount; ++descriptorSetIndex) {
		SpvReflectDescriptorSet* srcDescriptorSet = descriptorSets[descriptorSetIndex];
		auto& dstDescriptorSet = outArtifacts.reflectedDescriptorSets.emplace_back();
		dstDescriptorSet.setIndex = srcDescriptorSet->set;
		dstDescriptorSet.bindingStartIndex = static_cast<uint32_t>(outArtifacts.reflectedDescriptorBindings.size());
		dstDescriptorSet.bindingCount = srcDescriptorSet->binding_count;

		for (uint32_t descriptorBindingIndex = 0; descriptorBindingIndex < srcDescriptorSet->binding_count; ++descriptorBindingIndex) {
			SpvReflectDescriptorBinding* srcDescriptorBinding = srcDescriptorSet->bindings[descriptorBindingIndex];
			auto& dstDescriptorBinding = outArtifacts.reflectedDescriptorBindings.emplace_back();
			dstDescriptorBinding.bindingIndex = srcDescriptorBinding->binding;
			dstDescriptorBinding.count = srcDescriptorBinding->count;
			dstDescriptorBinding.stages = ToShaderStageBit(outArtifacts.stage);

			switch (srcDescriptorBinding->descriptor_type) {
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::CombinedImageSampler;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::UniformTexelBuffer;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::StorageTexelBuffer;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::StorageBuffer;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::StorageBufferDynamic;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::StorageImage;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::UniformBufferDynamic;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::StorageImage;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::Sampler;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::SampledImage;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::UniformBuffer;
				break;
			case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
				dstDescriptorBinding.type = Grindstone::GraphicsAPI::BindingType::AccelerationStructure;
				break;
			default:
				GS_ASSERT_LOG("Unsupported reflect descriptor binding type!");
			}
		}
	}

	spvReflectDestroyShaderModule(&module);
	
	return true;
}

Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;

static bool TranspileShader(LogCallback logCallback, CompilationOptions& options, std::string_view entrypoint, Grindstone::GraphicsAPI::ShaderStage stage, const std::filesystem::path& sourcePath, std::string inputCode, StageCompilationArtifacts& outArtifacts) {
	if (pUtils == nullptr) {
		::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
	}

	if (pCompiler == nullptr) {
		::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf()));
	}

	outArtifacts.stage = stage;

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource;
	pUtils->CreateBlob(inputCode.c_str(), static_cast<UINT32>(inputCode.size()), CP_UTF8, pSource.GetAddressOf());

	std::wstring entryPoint = std::wstring(entrypoint.begin(), entrypoint.end());
	constexpr std::array<LPCWSTR, Grindstone::GraphicsAPI::numShaderTotalStage> profiles = {
		L"vs_6_6",
		L"ds_6_6",
		L"hs_6_6",
		L"gs_6_6",
		L"ps_6_6",
		L"as_6_6",
		L"ms_6_6",
		L"cs_6_6"
	};

	LPCWSTR profile = profiles[static_cast<uint8_t>(stage)];

	std::vector<LPCWSTR> compilationArguments {
		L"-E",
		entryPoint.c_str(),
		L"-T",
		profile,
		L"-Wno-ignored-attributes"
	};

	if (options.target == CompilationOptions::Target::DirectX) {
		compilationArguments.push_back(L"-Qstrip_debug");
		compilationArguments.push_back(L"-Qstrip_reflect");
	}
	else {
		compilationArguments.push_back(L"-spirv");
		compilationArguments.push_back(L"-fspv-reflect");
		compilationArguments.push_back(L"-fspv-entrypoint-name=main");
		if (options.isDebug) {
			compilationArguments.push_back(L"-fspv-debug=vulkan-with-source");
		}
	}

	if (options.isDebug) {
		compilationArguments.push_back(DXC_ARG_DEBUG);
	}
	else {
		compilationArguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
	}

	DxcBuffer sourceBuffer{};
	sourceBuffer.Ptr = pSource->GetBufferPointer();
	sourceBuffer.Size = pSource->GetBufferSize();
	sourceBuffer.Encoding = 0u;

	IDxcIncludeHandler* pIncludeHandler = nullptr;

	// Compile the shader.
	Microsoft::WRL::ComPtr<IDxcResult> pResults{};
	if (FAILED(Compile(pCompiler.Get(), &sourceBuffer, compilationArguments.data(), static_cast<int>(compilationArguments.size()), pIncludeHandler, &pResults)))
	{
		// Either an unrecoverable error exception was caught or a failing HRESULT was returned
		// Use fputs to prevent any chance of new allocations
		// Terminate the process

		logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Output, fmt::format("Critical Error in {}", entrypoint), sourcePath, UNDEFINED_LINE, UNDEFINED_COLUMN);

		return false;
	}

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
	// Note that d3dcompiler would return null if no errors or warnings are present.
	// IDxcCompiler3::Compile will always return an error buffer,
	// but its length will be zero if there are no warnings or errors.
	if (
		SUCCEEDED(pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)) &&
		pErrors != nullptr &&
		pErrors->GetStringLength() != 0
	) {
		std::string outputStr = pErrors->GetStringPointer();
		PrintShaderMessage(logCallback, sourcePath, outputStr);
	}

	//
	// Quit if the compilation failed.
	//
	HRESULT hrStatus;
	if (FAILED(pResults->GetStatus(&hrStatus)) || FAILED(hrStatus)) {
		logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Output, "Could not compile shader.", sourcePath, UNDEFINED_LINE, UNDEFINED_COLUMN);
		return false;
	}

	if (options.target == CompilationOptions::Target::DirectX) {
		return GatherArtifactsDirectX(pUtils.Get(), pResults.Get(), outArtifacts);
	}
	else {
		return GatherArtifactsSpirV(pUtils.Get(), pResults.Get(), outArtifacts);
	}
	return false;
}

bool CompileShadersGraphics(LogCallback logCallback, const ResolvedStateTree::PipelineSet& pipelineSet, CompilationOptions& options, CompilationArtifactsGraphics& output) {
	bool hasError = false;

	for (const auto& configIterator : pipelineSet.configurations) {
		const ResolvedStateTree::Configuration& config = configIterator.second;
		CompilationArtifactsGraphics::Configuration& configArtifacts = output.configurations[configIterator.first];
		for (const auto& passIterator : config.passes) {
			const ResolvedStateTree::Pass& pass = passIterator.second;
			CompilationArtifactsGraphics::Pass& passArtifacts = configArtifacts.passes[configIterator.first];
			uint8_t stageIndex = 0;
			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				const std::string& stageEntrypoint = pass.stageEntryPoints[stageIndex];
				if (!stageEntrypoint.empty()) {
					Grindstone::GraphicsAPI::ShaderStage stageEnum = static_cast<Grindstone::GraphicsAPI::ShaderStage>(stageIndex++);
					StageCompilationArtifacts outArtifacts;
					if (TranspileShader(logCallback, options, stageEntrypoint, stageEnum, pipelineSet.sourceFilepath, pass.code, outArtifacts)) {
						passArtifacts.stages.emplace_back(outArtifacts);
					}
					else {
						hasError = true;
					}
				}
			}
		}
	}

	return !hasError;
}

bool CompileShadersCompute(LogCallback logCallback, const ResolvedStateTree::ComputeSet& computeSet, CompilationOptions& options, CompilationArtifactsCompute& output) {
	Grindstone::GraphicsAPI::ShaderStage stageEnum = Grindstone::GraphicsAPI::ShaderStage::Compute;
	StageCompilationArtifacts outArtifacts;
	if (TranspileShader(logCallback, options, computeSet.shaderEntrypoint, stageEnum, computeSet.sourceFilepath, computeSet.shaderCode, outArtifacts)) {
		output.computeStage = outArtifacts;
		return true;
	}
	else {
		return false;
	}
}
