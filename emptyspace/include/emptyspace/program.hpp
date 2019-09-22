#pragma once
#include <emptyspace/types.hpp>
#include <emptyspace/io.hpp>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <array>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

template <typename T>
u32 CreateShaderStorageBuffer(T* data, const u32 size)
{
    u32 name;
	glCreateBuffers(1, &name);
#if _DEBUG
	const auto label = typeid(T).name();
	glObjectLabel(GL_BUFFER, name, strlen(label), label);
#endif
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, name);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return name;
}

class Program
{
public:
	[[nodiscard]] u32 GetId() const
	{
		return _pipeline;
	}
	
	Program(const std::string_view vertexShaderFilePath, const std::string_view fragmentShaderFilePath)
	{
		auto const vertexShaderSource = ReadTextFile(vertexShaderFilePath);
		auto const fragmentShaderSource = ReadTextFile(fragmentShaderFilePath);

		auto const vertexShaderData = vertexShaderSource.data();
		auto const fragmentShaderData = fragmentShaderSource.data();

    	_vertexShader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShaderData);
		_fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShaderData);
#ifdef _DEBUG
		glObjectLabel(GL_PROGRAM, _vertexShader, vertexShaderFilePath.length(), vertexShaderFilePath.data());
		glObjectLabel(GL_PROGRAM, _fragmentShader, fragmentShaderFilePath.length(), fragmentShaderFilePath.data());
#endif
		ValidateProgram(_vertexShader, vertexShaderFilePath);
		ValidateProgram(_fragmentShader, fragmentShaderFilePath);

		glCreateProgramPipelines(1, &_pipeline);
		glUseProgramStages(_pipeline, GL_VERTEX_SHADER_BIT, _vertexShader);
		glUseProgramStages(_pipeline, GL_FRAGMENT_SHADER_BIT, _fragmentShader);
	}

    ~Program()
	{
		glDeleteProgramPipelines(1, &_pipeline);
		glDeleteProgram(_vertexShader);
		glDeleteProgram(_fragmentShader);
	}

	template <typename T>
	void SetFragmentShaderUniform(s32 location, T const& value)
	{
		SetProgramUniform(_fragmentShader, location, value);
	}
	
	template <typename T>
	void SetVertexShaderUniform(s32 location, T const& value)
	{
		SetProgramUniform(_vertexShader, location, value);
	}
		
	void Use() const
	{
		glBindProgramPipeline(_pipeline);
	}
	
private:
	template <typename T>
	void SetProgramUniform(u32 shader, s32 location, T const& value)
	{
		if constexpr (std::is_same_v<T, s32>)
		{
			glProgramUniform1i(shader, location, value);
		}
		else if constexpr (std::is_same_v<T, u32>)
		{
			glProgramUniform1ui(shader, location, value);
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			glProgramUniform1ui(shader, location, value);
		}
		else if constexpr (std::is_same_v<T, f32>)
		{
			glProgramUniform1f(shader, location, value);
		}
		else if constexpr (std::is_same_v<T, f64>)
		{
			glProgramUniform1d(shader, location, value);
		}
		else if constexpr (std::is_same_v<T, glm::vec2>)
		{
			glProgramUniform2fv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::vec3>)
		{
			glProgramUniform3fv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::vec4>)
		{
			glProgramUniform4fv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::ivec2>)
		{
			glProgramUniform2iv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::ivec3>)
		{
			glProgramUniform3iv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::ivec4>)
		{
			glProgramUniform4iv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::uvec2>)
		{
			glProgramUniform2uiv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::uvec3>)
		{
			glProgramUniform3uiv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::uvec4>)
		{
			glProgramUniform4uiv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::quat>)
		{
			glProgramUniform4fv(shader, location, 1, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::mat3>)
		{
			glProgramUniformMatrix3fv(shader, location, 1, GL_FALSE, glm::value_ptr(value));
		}
		else if constexpr (std::is_same_v<T, glm::mat4>)
		{
			glProgramUniformMatrix4fv(shader, location, 1, GL_FALSE, glm::value_ptr(value));
		}
		else throw std::runtime_error("unsupported type");
	}
	
	static void ValidateProgram(const u32 shader, const std::string_view filename)
	{
		s32 compiled = 0;
		glProgramParameteri(shader, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glGetProgramiv(shader, GL_LINK_STATUS, &compiled);
		if (compiled == GL_FALSE)
		{
			std::array<char, 1024> compilerLog{};
			glGetProgramInfoLog(shader, u32(compilerLog.size()), nullptr, compilerLog.data());
			glDeleteShader(shader);

			std::ostringstream message;
			message << "Shader " << filename << " contains error(s):\n\n" << compilerLog.data() << '\n';
			std::clog << message.str();
		}
	}
	
	u32 _pipeline{};
	u32 _vertexShader{};
	u32 _fragmentShader{};
};
