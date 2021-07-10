#include <iostream>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spirv-cross/spirv_cross.hpp>
#include <spirv-cross/spirv_glsl.hpp>
#include "ShaderImporter.hpp"

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

std::vector<uint32_t> compileVulkanGlslToSpirv(
	const std::string& source_name,
	shaderc_shader_kind kind,
	const std::string& source,
	bool optimize = false
) {
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::SpvCompilationResult module =
		compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

	if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cerr << module.GetErrorMessage();
		return std::vector<uint32_t>();
	}

	return { module.cbegin(), module.cend() };
}

namespace Grindstone {
	namespace Converters {
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

			for (const auto& resource : resources.uniform_buffers) {
				const auto& bufferType = compiler.get_type(resource.base_type_id);
				uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				int memberCount = bufferType.member_types.size();
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