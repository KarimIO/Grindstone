#include <GL/gl3w.h>
#include "GLUniformBuffer.hpp"
#include "GLTexture.hpp"
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "GLPipeline.hpp"
#include "GLCore.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		GLPipeline::GLPipeline(CreateInfo& createInfo) {
			CreatePipeline(createInfo);
		}

		void GLPipeline::Recreate(CreateInfo& createInfo) {
			glUseProgram(0);
			glDeleteProgram(program);

			CreatePipeline(createInfo);
		}

		void GLPipeline::CreatePipeline(CreateInfo& createInfo) {
			primitiveType = GetGeomType(createInfo.primitiveType);

			width = createInfo.width;
			height = createInfo.height;
			scissorWidth = createInfo.scissorW;
			scissorHeight = createInfo.scissorH;
			scissorX = createInfo.scissorX;
			scissorY = createInfo.scissorY;
			cullMode = createInfo.cullMode;

			program = glCreateProgram();
			glObjectLabel(GL_PROGRAM, program, -1, createInfo.shaderName);

			uint32_t shaderNum = createInfo.shaderStageCreateInfoCount;
			GLuint* shaders = new GLuint[shaderNum];
			for (uint32_t i = 0; i < shaderNum; i++) {
				shaders[i] = CreateShaderModule(createInfo.shaderStageCreateInfos[i]);
			}

			GLint result = 0;
			glLinkProgram(program);

			GLint isLinked;
			glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
			if (isLinked == GL_FALSE) {
				GLsizei infoLength;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
				std::vector<char> programLinkErrorMessage(infoLength + 1);
				glGetProgramInfoLog(program, infoLength, NULL, programLinkErrorMessage.data());
				printf("%s\n", programLinkErrorMessage.data());
			}

			for (size_t i = 0; i < shaderNum; i++) {
				glDeleteShader(shaders[i]);
			}

			delete[] shaders;
		}

		GLuint GLPipeline::CreateShaderModule(ShaderStageCreateInfo createInfo) {
			int shaderType;
			switch (createInfo.type) {
			default:
			case ShaderStage::Vertex:
				shaderType = GL_VERTEX_SHADER;
				break;
			case ShaderStage::Fragment:
				shaderType = GL_FRAGMENT_SHADER;
				break;
			case ShaderStage::TesselationEvaluation:
				shaderType = GL_TESS_EVALUATION_SHADER;
				break;
			case ShaderStage::TesselationControl:
				shaderType = GL_TESS_CONTROL_SHADER;
				break;
			case ShaderStage::Geometry:
				shaderType = GL_GEOMETRY_SHADER;
				break;
			}

			GLuint shader = glCreateShader(shaderType);
			if (createInfo.fileName != nullptr) {
				glObjectLabel(GL_SHADER, shader, -1, createInfo.fileName);
			}

			bool shouldUseTextShaders = false;

			if (shouldUseTextShaders) {
				const GLint size = createInfo.size;
				glShaderSource(shader, 1, &createInfo.content, &size);
				glCompileShader(shader);

				GLint result = GL_FALSE;
				int infoLength;

				// Check Shader
				glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
				if (!result) {
					printf("Error Report in Shader %s\n", createInfo.fileName);
					std::vector<char> VertexShaderErrorMessage(infoLength + 1);
					glGetShaderInfoLog(shader, infoLength, NULL, VertexShaderErrorMessage.data());
					printf("%s\n", VertexShaderErrorMessage.data());
					return false;
				}
			}
			else {
				glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, createInfo.content, createInfo.size);
				glSpecializeShader(shader, "main", 0, 0, 0);
			}

			glAttachShader(program, shader);

			return shader;
		}

		void GLPipeline::Bind() {
			glUseProgram(program);

			// glViewport(0, 0, (GLsizei)width, (GLsizei)height);
			// glScissor(scissor_x_, scissor_y_, scissor_w_, scissor_h_);
			switch (cullMode) {
			case CullMode::None:
				glDisable(GL_CULL_FACE);
				break;
			case CullMode::Front:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			case CullMode::Back:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			case CullMode::Both:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT_AND_BACK);
				break;
			}
		}

		GLuint GLPipeline::GetPrimitiveType() {
			return primitiveType;
		}

		GLPipeline::~GLPipeline() {
			glUseProgram(0);
			glDeleteProgram(program);
		}
	}
}
