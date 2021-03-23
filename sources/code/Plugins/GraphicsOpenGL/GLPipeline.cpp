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
		GLuint GLPipeline::createShaderModule(ShaderStageCreateInfo createInfo) {
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

			const GLint length = createInfo.size;

			std::string test;
			test.reserve(length);
			std::memcpy((void *)test.data(), createInfo.content, length);

			glShaderSource(shader, 1, &createInfo.content, &length);
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

			glAttachShader(program_, shader);

			return shader;
		}

		GLPipeline::GLPipeline(CreateInfo& createInfo) {
			primitive_type_ = GetGeomType(createInfo.primitiveType);

			width_ = createInfo.width;
			height_ = createInfo.height;
			scissor_w_ = createInfo.scissorW;
			scissor_h_ = createInfo.scissorH;
			scissor_x_ = createInfo.scissorX;
			scissor_y_ = createInfo.scissorY;
			cull_mode_ = createInfo.cullMode;

			program_ = glCreateProgram();

			uint32_t shaderNum = createInfo.shaderStageCreateInfoCount;
			GLuint *shaders = new GLuint[shaderNum];
			for (uint32_t i = 0; i < shaderNum; i++) {
				shaders[i] = createShaderModule(createInfo.shaderStageCreateInfos[i]);
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

			auto& vbd = createInfo.vertex_bindings;
			for (uint32_t i = 0; i < vbd->attribute_count; i++) {
				glBindAttribLocation(program_, vbd->attributes[i].location, vbd->attributes[i].name);
			}

			GLint result = 0;
			glLinkProgram(program_);
			glGetProgramiv(program_, GL_LINK_STATUS, &result);
			if (!result) {
				int maxLength = 0;
				glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(program_, maxLength, &maxLength, infoLog.data());
				fprintf(stderr, "Error linking shader program: '%s'\n", infoLog.data());
				return;
			}

			// Detach Shaders (Remove their references)
			for (size_t i = 0; i < shaderNum; i++) {
				glDetachShader(program_, shaders[i]);
				glDeleteShader(shaders[i]);
			}

			delete[] shaders;

			// TODO: Properly do this:
			glUseProgram(program_);

			for (size_t i = 0; i < createInfo.uniformBufferBindingCount; i++) {
				GLUniformBufferBinding *ubb = (GLUniformBufferBinding *)createInfo.uniformBufferBindings[i];
				GLuint index = glGetUniformBlockIndex(program_, ubb->GetUniformName());
				if (index != GL_INVALID_INDEX) {
					glUniformBlockBinding(program_, index, ubb->GetBindingLocation());
				}
				else {
					std::cout << "Couldn't attach Uniform Buffer" << ubb->GetUniformName() << std::endl;
				}
			}

			for (uint32_t i = 0; i < createInfo.textureBindingCount; i++) {
				GLTextureBindingLayout *texbinding = (GLTextureBindingLayout *)createInfo.textureBindings[i];
				for (uint32_t j = 0; j < texbinding->getNumSubBindings(); j++) {
					TextureSubBinding sub = texbinding->getSubBinding(j);
					int loc = glGetUniformLocation(program_, sub.shaderLocation);
					glUniform1i(loc, sub.textureLocation);
				}
			}
		}

		void GLPipeline::bind() {
			glUseProgram(program_);

			glViewport(0, 0, (GLsizei)width_, (GLsizei)height_);
			glScissor(scissor_x_, scissor_y_, scissor_w_, scissor_h_);
			switch (cull_mode_) {
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

		GLuint GLPipeline::getPrimitiveType() {
			return primitive_type_;
		}

		GLPipeline::~GLPipeline() {
			glUseProgram(0);
			glDeleteProgram(program_);
		}
	}
}