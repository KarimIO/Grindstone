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
		throw std::runtime_error(std::string("Failed to open file ") + filename);
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

	if (!ifs.is_open()) {
		throw std::runtime_error(std::string("Failed to open file ") + filename);
	}

	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return content;
}

void reflectImages(
	Grindstone::Converters::ShaderImporter::ShaderType shaderType,
	std::vector<Grindstone::Converters::ShaderImporter::Texture>& textures,
	spirv_cross::Compiler& compiler,
	spirv_cross::SmallVector<spirv_cross::Resource>& resourceList
) {
	for (const auto& resource : resourceList) {
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		auto resourceName = resource.name;
		textures.emplace_back(resourceName, binding);
		textures.back().shaderPasses.push_back(shaderType);
	}
}

void reflectStruct(
	Grindstone::Converters::ShaderImporter::ShaderType shaderType,
	std::vector<Grindstone::Converters::ShaderImporter::UniformBuffer>& uniformBuffers,
	spirv_cross::Compiler& compiler,
	spirv_cross::SmallVector<spirv_cross::Resource>& resourceList
) {
	for (const auto& resource : resourceList) {
		const auto& bufferType = compiler.get_type(resource.base_type_id);
		uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		int memberCount = bufferType.member_types.size();
		auto resourceName = resource.name;

		uniformBuffers.emplace_back(resourceName.c_str(), binding, bufferSize);
		auto& uniformBuffer = uniformBuffers.back();
		uniformBuffer.shaderPasses.push_back(shaderType);

		for (unsigned i = 0; i < memberCount; i++) {
			auto& memberType = compiler.get_type(bufferType.member_types[i]);
			size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
			size_t offset = compiler.type_struct_member_offset(bufferType, i);
			const std::string& name = compiler.get_member_name(bufferType.self, i);
			auto& typeStr = getDataTypeName(memberType.basetype);
			memberType.vecsize;
			memberType.columns;

			uniformBuffer.members.emplace_back(name, typeStr, offset, memberSize);
		}
	}
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

		ShaderImporter::ShaderImporter() : reflectionWriter(reflectionStringBuffer) {}

		void ShaderImporter::convertFile(const char* filePath) {
			try {
				path = filePath;

				basePath = path;
				auto lastPeriod = path.find_last_of('.');
				if (lastPeriod != -1) {
					basePath = path.substr(0, lastPeriod);
				}

				sourceFileContents = readTextFile(filePath);

				process();
			}
			catch (std::runtime_error& e) {
				std::cerr << e.what() << std::endl;
			}
		}

		void ShaderImporter::process() {
			shaderName = extractField("#name");
			renderQueue = extractField("#renderQueue");
			extractSubmodules();
			writeReflectionDocument();
		}
		void ShaderImporter::writeReflectionStruct(std::vector<UniformBuffer>& structs) {
			reflectionWriter.StartArray();
			for each (auto & structMeta in structs) {
				reflectionWriter.StartObject();
				reflectionWriter.Key("name");
				reflectionWriter.String(structMeta.name.c_str());
				reflectionWriter.Key("binding");
				reflectionWriter.Uint(structMeta.binding);
				reflectionWriter.Key("bufferSize");
				reflectionWriter.Uint(structMeta.buffserSize);

				reflectionWriter.Key("usedIn");
				reflectionWriter.StartArray();
				for each (auto & shaderPass in structMeta.shaderPasses) {
					reflectionWriter.String(getShaderTypeAsString(shaderPass));
				}
				reflectionWriter.EndArray();

				reflectionWriter.Key("members");
				reflectionWriter.StartArray();
				for each (auto & member in structMeta.members) {
					reflectionWriter.StartObject();
					reflectionWriter.Key("name");
					reflectionWriter.String(member.name.c_str());
					reflectionWriter.Key("offset");
					reflectionWriter.Uint(member.offset);
					reflectionWriter.Key("memberSize");
					reflectionWriter.Uint(member.memberSize);
					reflectionWriter.Key("type");
					reflectionWriter.String(member.type.c_str());
					reflectionWriter.EndObject();
				}
				reflectionWriter.EndArray();

				reflectionWriter.EndObject();
			}
			reflectionWriter.EndArray();
		}

		void ShaderImporter::writeReflectionImage(std::vector<Texture>& resources) {
			reflectionWriter.StartArray();
			for each (auto & resource in resources) {
				reflectionWriter.StartObject();
				reflectionWriter.Key("name");
				reflectionWriter.String(resource.name.c_str());
				reflectionWriter.Key("binding");
				reflectionWriter.Uint(resource.binding);

				reflectionWriter.Key("usedIn");
				reflectionWriter.StartArray();
				for each (auto & shaderPass in resource.shaderPasses) {
					reflectionWriter.String(getShaderTypeAsString(shaderPass));
				}
				reflectionWriter.EndArray();

				reflectionWriter.EndObject();
			}
			reflectionWriter.EndArray();
		}

		void ShaderImporter::writeReflectionDocument() {
			reflectionWriter.StartObject();

			reflectionWriter.Key("name");
			reflectionWriter.String(shaderName.c_str());

			reflectionWriter.Key("renderQueue");
			reflectionWriter.String(renderQueue.c_str());

			reflectionWriter.Key("shaderModules");
			reflectionWriter.StartArray();
			for each (auto & shaderPass in shaderPasses) {
				reflectionWriter.String(getShaderTypeAsString(shaderPass));
			}
			reflectionWriter.EndArray();

			reflectionWriter.Key("uniformBuffers");
			writeReflectionStruct(uniformBuffers);

			reflectionWriter.Key("textures");
			writeReflectionImage(textures);

			reflectionWriter.Key("samplers");
			writeReflectionImage(samplers);

			reflectionWriter.EndObject();

			outputStringToFile(".reflect.json", reflectionStringBuffer.GetString());
		}

		std::string ShaderImporter::extractField(const char* fieldKey) {
			auto fieldPos = sourceFileContents.find(fieldKey);
			auto newLinePos = sourceFileContents.find('\n', fieldPos);
			auto valuePos = fieldPos + strlen(fieldKey) + 1;
			return sourceFileContents.substr(valuePos, newLinePos - valuePos);
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
				auto opengGlsl = convertToOpenglGlsl(extension, vkSpirv);
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
				throw std::runtime_error(result.GetErrorMessage());
			}

			auto vkSpirv = std::vector<uint32_t>(result.cbegin(), result.cend());

			outputUint32ToFile((std::string(extension) + ".vulkan.spv").c_str(), vkSpirv);

			return vkSpirv;
		}

		std::string ShaderImporter::convertToOpenglGlsl(const char* extension, std::vector<uint32_t>& vkSpirv) {
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
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

			auto result = compiler.CompileGlslToSpv(
				opengGlsl,
				getShaderTypeForShaderc(shaderType),
				path.c_str(),
				options
			);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				throw std::runtime_error(result.GetErrorMessage());
			}

			outputUint32ToFile((std::string(extension) + ".opengl.spv").c_str(), std::vector<uint32_t>(result.cbegin(), result.cend()));
		}

		void ShaderImporter::reflectResources(ShaderType shaderType, std::vector<uint32_t>& vkSpirv) {
			spirv_cross::Compiler compiler(vkSpirv);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			shaderPasses.push_back(shaderType);

			std::string resourcesStr;
			if (resources.uniform_buffers.size()) {
				reflectStruct(shaderType, uniformBuffers, compiler, resources.uniform_buffers);
			}
			if (resources.push_constant_buffers.size()) {
				// reflectStruct(shaderType, pushConstants, compiler, resources.push_constant_buffers);
			}
			if (resources.sampled_images.size()) {
				reflectImages(shaderType, samplers, compiler, resources.sampled_images);
			}
			if (resources.separate_images.size()) {
				reflectImages(shaderType, textures, compiler, resources.separate_images);
			}
		}

		void ShaderImporter::outputStringToFile(const char* extension, const char* content) {
			std::string outputFilename = basePath + extension;
			std::ofstream file(outputFilename);
			file.write((const char*)content, strlen(content));
			file.flush();
			file.close();
		}

		void ShaderImporter::outputUint32ToFile(const char* extension, std::vector<uint32_t>& content) {
			std::string outputFilename = basePath + extension;
			std::ofstream file(outputFilename, std::ios::out | std::ios::binary);
			auto fileSize = sizeof(uint32_t) * content.size();
			file.write((const char*)content.data(), fileSize);
			file.flush();
			file.close();
		}
	}
}