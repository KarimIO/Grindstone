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

				// int compiled = 0;
				// glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			}

			glAttachShader(program, shader);

			return shader;
		}

		GLPipeline::GLPipeline(CreateInfo& createInfo) {
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
			GLuint *shaders = new GLuint[shaderNum];
			for (uint32_t i = 0; i < shaderNum; i++) {
				shaders[i] = CreateShaderModule(createInfo.shaderStageCreateInfos[i]);
			}

			/*glValidateProgram(program);
			glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
			if (!result) {
				int maxLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
				fprintf(stderr, "Invalid shader program: '%s'\n", infoLog.data());
				//return;
			}*/

			/*if (createInfo.vertexBindingsCount > 0) {
				auto& vbd = createInfo.vertexBindings;
				for (uint32_t i = 0; i < vbd->attributeCount; i++) {
					glBindAttribLocation(program, vbd->attributes[i].location, vbd->attributes[i].name);
				}
			}*/

			GLint result = 0;
			glLinkProgram(program);

			GLint isLinked;
			glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
			if (isLinked == GL_FALSE) {
				printf("Link failed for program: %s", createInfo.shaderName);
				/*GLint maxLength;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
				printf("Shader linking failed %s\n", infoLog.data());

				glDeleteProgram(program);*/
			}

			// Detach Shaders (Remove their references)
			for (size_t i = 0; i < shaderNum; i++) {
				// glDetachShader(program, shaders[i]);
				glDeleteShader(shaders[i]);
			}

			delete[] shaders;

			// TODO: Properly do this:
			// glUseProgram(program);

			/*for (size_t i = 0; i < createInfo.uniformBufferBindingCount; i++) {
				GLUniformBufferBinding *ubb = (GLUniformBufferBinding *)createInfo.uniformBufferBindings[i];
				glUniformBlockBinding(program, ubb->GetBindingLocation(), ubb->GetBindingLocation());
				GLuint index = glGetUniformBlockIndex(program, ubb->GetUniformName());
				if (index != GL_INVALID_INDEX) {
				}
				else {
					std::cout << "Couldn't attach Uniform Buffer" << ubb->GetUniformName() << std::endl;
				}
			}*/

			/*for (uint32_t i = 0; i < createInfo.textureBindingCount; i++) {
				GLTextureBindingLayout *texbinding = (GLTextureBindingLayout *)createInfo.textureBindings[i];
				for (uint32_t j = 0; j < texbinding->getNumSubBindings(); j++) {
					TextureSubBinding sub = texbinding->getSubBinding(j);
					int loc = glGetUniformLocation(program, sub.shaderLocation);
					glUniform1i(loc, sub.textureLocation);
				}
			}*/
		}

		void GLPipeline::Bind() {
			glUseProgram(program);

			glViewport(0, 0, (GLsizei)width, (GLsizei)height);
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