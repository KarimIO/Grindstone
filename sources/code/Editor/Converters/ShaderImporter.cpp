#include <iostream>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spirv-cross/spirv_cross.hpp>
#include <spirv-cross/spirv_glsl.hpp>
#include "ShaderImporter.hpp"

std::string getDataTypeName(spirv_cross::SPIRType::BaseType type) {
	switch (type) {
		case spirv_cross::SPIRType::BaseType::Void: return "Void";
		case spirv_cross::SPIRType::BaseType::Boolean: return "Boolean";
		case spirv_cross::SPIRType::BaseType::SByte: return "SByte";
		case spirv_cross::SPIRType::BaseType::UByte: return "UByte";
		case spirv_cross::SPIRType::BaseType::Short: return "Short";
		case spirv_cross::SPIRType::BaseType::UShort: return "UShort";
		case spirv_cross::SPIRType::BaseType::Int: return "Int";
		case spirv_cross::SPIRType::BaseType::UInt: return "UInt";
		case spirv_cross::SPIRType::BaseType::Int64: return "Int64";
		case spirv_cross::SPIRType::BaseType::UInt64: return "UInt64";
		case spirv_cross::SPIRType::BaseType::AtomicCounter: return "AtomicCounter";
		case spirv_cross::SPIRType::BaseType::Half: return "Half";
		case spirv_cross::SPIRType::BaseType::Float: return "Float";
		case spirv_cross::SPIRType::BaseType::Double: return "Double";
		case spirv_cross::SPIRType::BaseType::Struct: return "Struct";
		case spirv_cross::SPIRType::BaseType::Image: return "Image";
		case spirv_cross::SPIRType::BaseType::SampledImage: return "SampledImage";
		case spirv_cross::SPIRType::BaseType::Sampler: return "Sampler";
		case spirv_cross::SPIRType::BaseType::AccelerationStructure: return "AccelerationStructure";
		case spirv_cross::SPIRType::BaseType::RayQuery: return "RayQuery";
		default: return "Unknown";
	}
}

std::vector<char> readBinaryFile(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

std::string readTextFile(const char* filename) {
	std::ifstream ifs(filename);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return content;
}

std::string reflectResourceType(
	const char* resourceType,
	spirv_cross::Compiler& compiler,
	spirv_cross::SmallVector<spirv_cross::Resource>& resourceList
) {
	std::string output = std::string("\n         ") + resourceType + ":";
	for (const auto& resource : resourceList) {
		const auto& bufferType = compiler.get_type(resource.base_type_id);
		uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		int memberCount = bufferType.member_types.size();
		auto resourceName = resource.name;

		output += "\n          - name: " + resourceName + "\n";
		output += "            binding: " + std::to_string(binding) + "\n";
		output += "            bufferSize: " + std::to_string(bufferSize) + "\n";
		output += "            members:";
		unsigned member_count = bufferType.member_types.size();
		for (unsigned i = 0; i < member_count; i++) {
			auto& member_type = compiler.get_type(bufferType.member_types[i]);
			size_t member_size = compiler.get_declared_struct_member_size(bufferType, i);

			// Get member offset within this struct.
			size_t offset = compiler.type_struct_member_offset(bufferType, i);

			if (!member_type.array.empty()) {
				// Get array stride, e.g. float4 foo[]; Will have array stride of 16 bytes.
				size_t array_stride = compiler.type_struct_member_array_stride(bufferType, i);
			}

			if (member_type.columns > 1) {
				// Get bytes stride between columns (if column major), for float4x4 -> 16 bytes.
				size_t matrix_stride = compiler.type_struct_member_matrix_stride(bufferType, i);
			}
			const std::string& name = compiler.get_member_name(bufferType.self, i);
			output += "\n             - name: " + name + "\n";
			output += "               offset: " + std::to_string(offset) + "\n";
			output += "               type: " + getDataTypeName(member_type.basetype) + "\n";
			output += "               memberSize: " + std::to_string(member_size);
		}
	}

	return output;
}

namespace Grindstone {
	namespace Converters {
		void ImportShadersFromGlsl(const char* filePath) {
			Converters::ShaderImporter importer;
			importer.convertFile(filePath);
		}

		shaderc_shader_kind getShaderTypeForShaderc(ShaderImporter::ShaderType type) {
			switch(type) {
			case ShaderImporter::ShaderType::Vertex: return shaderc_glsl_vertex_shader;
			case ShaderImporter::ShaderType::Fragment: return shaderc_glsl_fragment_shader;
			case ShaderImporter::ShaderType::Geometry: return shaderc_glsl_geometry_shader;
			case ShaderImporter::ShaderType::Compute: return shaderc_glsl_compute_shader;
			case ShaderImporter::ShaderType::TesselationControl: return shaderc_glsl_tess_control_shader;
			case ShaderImporter::ShaderType::TesselationEvaluation: return shaderc_glsl_tess_evaluation_shader;
			}
		}

		void ShaderImporter::convertFile(const char* filePath) {
			path = filePath;

			basePath = path;
			auto lastPeriod = path.find_last_of('.');
			if (lastPeriod != -1) {
				basePath = path.substr(0, lastPeriod);
			}

			sourceFileContents = readTextFile(filePath);

			process();
		}

		void ShaderImporter::process() {
			extractName();
			reflectionFileOutput = "shader:\n    - name: " + shaderName;
			extractSubmodules();

			outputStringToFile(".reflect.yml", reflectionFileOutput);
		}

		std::string ShaderImporter::extractField(const char* fieldKey) {
			auto fieldPos = sourceFileContents.find(fieldKey);
			auto newLinePos = sourceFileContents.find('\n', fieldPos);
			auto valuePos = fieldPos + strlen(fieldKey) + 1;
			return sourceFileContents.substr(valuePos, newLinePos - valuePos);
		}

		void ShaderImporter::extractName() {
			shaderName = extractField("#name");
		}
		
		void ShaderImporter::extractSubmodules() {
			const char* fieldKey = "#shaderModule";
			const size_t fieldKeyLength = strlen(fieldKey) + 1;

			size_t beginSearchPos = 0;

			while (true) {
				size_t fieldPos = sourceFileContents.find(fieldKey, beginSearchPos);
				if (fieldPos == -1) {
					return;
				}

				size_t newLinePos = sourceFileContents.find('\n', fieldPos);
				size_t valuePos = fieldPos + fieldKeyLength;

				size_t beginPos = newLinePos + 1;
				size_t endPos = sourceFileContents.find("#endShaderModule", newLinePos);

				std::string shaderTypeString = sourceFileContents.substr(valuePos, newLinePos - valuePos);
				auto shaderType = getShaderTypeFromString(shaderTypeString);
				std::string glsl = sourceFileContents.substr(beginPos, endPos - beginPos);
				std::string extension = getShaderTypeExtension(shaderType);
				processSubmodule(shaderType, extension.c_str(), glsl.c_str());

				beginSearchPos = endPos;
			}
		}

		ShaderImporter::ShaderType ShaderImporter::getShaderTypeFromString(std::string& str) {
			if (str == "vertex") {
				return ShaderType::Vertex;
			}
			else if (str == "fragment") {
				return ShaderType::Fragment;
			}
			else if (str == "geometry") {
				return ShaderType::Geometry;
			}
			else if (str == "compute") {
				return ShaderType::Compute;
			}
			else if (str == "tesselationControl") {
				return ShaderType::TesselationControl;
			}
			else if (str == "tesselationEvaluation") {
				return ShaderType::TesselationEvaluation;
			}

			throw std::runtime_error("Invalid Shader Type!");
		}

		const char* ShaderImporter::getShaderTypeExtension(ShaderType type) {
			switch(type) {
			case ShaderType::Vertex: return ".vert";
			case ShaderType::Fragment: return ".frag";
			case ShaderType::Geometry: return ".geom";
			case ShaderType::Compute: return ".comp";
			case ShaderType::TesselationControl: return ".ctrl";
			case ShaderType::TesselationEvaluation: return ".eval";
			}
		}
		
		const char* ShaderImporter::getShaderTypeAsString(ShaderType type) {
			switch(type) {
			case ShaderType::Vertex: return "vertex";
			case ShaderType::Fragment: return "fragment";
			case ShaderType::Geometry: return "geometry";
			case ShaderType::Compute: return "compute";
			case ShaderType::TesselationControl: return "tesselationControl";
			case ShaderType::TesselationEvaluation: return "tesselationEvaluation";
			}
		}

		void ShaderImporter::processSubmodule(ShaderType shaderType, const char* extension, const char* glslSource) {
			std::vector<uint32_t> vkSpirv = convertToSpirv(shaderType, extension, glslSource);
			{
				auto opengGlsl = convertToOpenglGlsl(vkSpirv);
				convertToOpenglSpirv(shaderType, extension, glslSource);
			}
			reflectResources(shaderType, vkSpirv);
		}

		std::vector<uint32_t> ShaderImporter::convertToSpirv(ShaderType shaderType, const char* extension, const char* shaderModuleGlsl) {
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;

			auto result = compiler.CompileGlslToSpv(
				shaderModuleGlsl,
				getShaderTypeForShaderc(shaderType),
				path.c_str(),
				options
			);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				std::cerr << result.GetErrorMessage() << std::endl;
			}

			auto vkSpirv = std::vector<uint32_t>(result.cbegin(), result.cend());

			outputUint32ToFile((std::string(extension) + ".vulkan.spv").c_str(), vkSpirv);

			return vkSpirv;
		}

		std::string ShaderImporter::convertToOpenglGlsl(std::vector<uint32_t>& vkSpirv) {
			spirv_cross::CompilerGLSL glslTranspiler(vkSpirv);

			spirv_cross::CompilerGLSL::Options glslTranspilerOptions;
			glslTranspilerOptions.version = 450;
			glslTranspiler.set_common_options(glslTranspilerOptions);

			std::string openglGlsl = glslTranspiler.compile();

			return openglGlsl;
		}

		void ShaderImporter::convertToOpenglSpirv(ShaderType shaderType, const char* extension, const char* opengGlsl) {
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

			auto result = compiler.CompileGlslToSpv(
				opengGlsl,
				getShaderTypeForShaderc(shaderType),
				path.c_str(),
				options
			);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				std::cerr << result.GetErrorMessage() << std::endl;
			}

			outputUint32ToFile((std::string(extension) + ".opengl.spv").c_str(), std::vector<uint32_t>(result.cbegin(), result.cend()));
		}

		void ShaderImporter::reflectResources(ShaderType shaderType, std::vector<uint32_t>& vkSpirv) {
			spirv_cross::Compiler compiler(vkSpirv);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();
			auto shaderTypeStr = getShaderTypeAsString(shaderType);

			reflectionFileOutput += std::string("\n    - ") + shaderTypeStr;

			std::string resourcesStr;
			if (resources.uniform_buffers.size()) {
				resourcesStr += reflectResourceType("uniformBuffers", compiler, resources.uniform_buffers);
			}
			if (resources.push_constant_buffers.size()) {
				resourcesStr += reflectResourceType("pushConstants", compiler, resources.push_constant_buffers);
			}
			if (resources.separate_images.size()) {
				resourcesStr += reflectResourceType("texture2ds", compiler, resources.separate_images);
			}

			if (!resourcesStr.empty()) {
				reflectionFileOutput += ": " + resourcesStr;
			}
			else {
				reflectionFileOutput += ": {}";
			}
		}

		void ShaderImporter::outputStringToFile(const char* extension, std::string& content) {
			std::string outputFilename = basePath + extension;
			std::ofstream file(outputFilename);
			file.write((const char*)content.data(), content.size());
			file.flush();
			file.close();
		}

		void ShaderImporter::outputUint32ToFile(const char* extension, std::vector<uint32_t>& content) {
			std::string outputFilename = basePath + extension;
			std::ofstream file(outputFilename);
			file.write((const char*)content.data(), sizeof(uint32_t) * content.size());
			file.flush();
			file.close();
		}
	}
}