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
		default: "Unknown";
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
	std::string output = std::string(",\n\t\"") + resourceType + "\": [";
	bool isFirstOfType = true;
	for (const auto& resource : resourceList) {
		const auto& bufferType = compiler.get_type(resource.base_type_id);
		uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		int memberCount = bufferType.member_types.size();
		auto resourceName = resource.name;

		if (!isFirstOfType) {
			output += ",\n";
		}
		else {
			isFirstOfType = false;
			output += "\n";
		}
		output += "\t\t{\n";
		output += "\t\t\t\"name\": \"" + resourceName + "\",\n";
		output += "\t\t\t\"binding\": " + std::to_string(binding) + ",\n";
		output += "\t\t\t\"bufferSize\": " + std::to_string(bufferSize) + ",\n";
		output += "\t\t\t\"members\": [";
		unsigned member_count = bufferType.member_types.size();
		bool isFirstMember = true;
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
			if (!isFirstMember) {
				output += ",\n";
			}
			else {
				isFirstMember = false;
				output += "\n";
			}
			output += "\t\t\t\t{\n";
			output += "\t\t\t\t\t\"name\": \"" + name + "\",\n";
			output += "\t\t\t\t\t\"offset\": \"" + std::to_string(offset) + "\",\n";
			output += "\t\t\t\t\t\"type\": \"" + getDataTypeName(member_type.basetype) + "\",\n";
			output += "\t\t\t\t\t\"memberSize\": \"" + std::to_string(member_size) + "\"\n";
			output += "\t\t\t\t}";
		}
		output += "\n\t\t\t]\n";
		output += "\t\t}";
	}

	return output + "\n\t]";
}

namespace Grindstone {
	namespace Converters {
		void ImportShadersFromGlsl(const char* filePath) {
			Converters::ShaderImporter importer;
			importer.convertFile(filePath);
		}

		void ShaderImporter::convertFile(const char* filePath) {
			path = filePath;

			basePath = path;
			auto lastPeriod = path.find_last_of('.');
			if (lastPeriod != -1) {
				basePath = path.substr(0, lastPeriod);
			}

			vkGlslSource = readTextFile(filePath);

			process();
		}

		void ShaderImporter::process() {
			convertToSpirv();
			{
				auto glsl = convertToOpenglGlsl();
				convertToOpenglSpirv(glsl);
			}
			reflectResources();
		}

		void ShaderImporter::convertToSpirv() {
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;

			auto result = compiler.CompileGlslToSpv(
				vkGlslSource,
				shaderc_glsl_vertex_shader,
				path.c_str(),
				options
			);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				std::cerr << result.GetErrorMessage() << std::endl;
			}

			vkSpirv = std::vector<uint32_t>(result.cbegin(), result.cend());

			outputUint32ToFile(".vulkan.spv", vkSpirv);
		}

		std::string ShaderImporter::convertToOpenglGlsl() {
			spirv_cross::CompilerGLSL glslTranspiler(vkSpirv);

			spirv_cross::CompilerGLSL::Options glslTranspilerOptions;
			glslTranspilerOptions.version = 450;
			glslTranspiler.set_common_options(glslTranspilerOptions);

			std::string openglGlsl = glslTranspiler.compile();

			return openglGlsl;
		}

		void ShaderImporter::convertToOpenglSpirv(std::string & glsl) {
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

			auto result = compiler.CompileGlslToSpv(
				glsl,
				shaderc_glsl_vertex_shader,
				path.c_str(),
				options
			);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				std::cerr << result.GetErrorMessage() << std::endl;
			}

			outputUint32ToFile(".opengl.spv", std::vector<uint32_t>(result.cbegin(), result.cend()));
		}

		void ShaderImporter::reflectResources() {
			spirv_cross::Compiler compiler(vkSpirv);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			std::string reflFileContents = "{\n";
			reflFileContents += "\t\"originalPath\": \"" + path + "\"";

			if (resources.uniform_buffers.size()) {
				reflFileContents += reflectResourceType("uniformBuffers", compiler, resources.uniform_buffers);
			}
			if (resources.push_constant_buffers.size()) {
				reflFileContents += reflectResourceType("pushConstants", compiler, resources.push_constant_buffers);
			}
			if (resources.separate_images.size()) {
				reflFileContents += reflectResourceType("texture2ds", compiler, resources.separate_images);
			}

			reflFileContents += "\n}";

			outputStringToFile(".reflect.json", reflFileContents);
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