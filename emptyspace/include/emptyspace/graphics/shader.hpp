#pragma once

#include <glad/glad.h>
#include <glm/packing.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	u32 Program;

	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
	{

		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{

			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;

			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure& exception)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
		}
		auto vShaderCode = vertexCode.c_str();
		auto fShaderCode = fragmentCode.c_str();
		

		u32 vertexShader;
		u32 fragmentShader;

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
		glCompileShader(vertexShader);
		CheckCompileErrors(vertexShader, "VERTEX");

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
		glCompileShader(fragmentShader);
		CheckCompileErrors(fragmentShader, "FRAGMENT");

		unsigned int geometry;
		if (geometryPath != nullptr)
		{
			auto gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, nullptr);
			glCompileShader(geometry);
			CheckCompileErrors(geometry, "GEOMETRY");
		}
		
		Program = glCreateProgram();
		glAttachShader(Program, vertexShader);
		glAttachShader(Program, fragmentShader);
		if (geometryPath != nullptr)
		{
			glAttachShader(Program, geometry);
		}
		glLinkProgram(Program);
		CheckCompileErrors(Program, "PROGRAM");

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		if (geometryPath != nullptr)
		{
			glDeleteShader(geometry);
		}

	}

	void Use()
	{
		glUseProgram(Program);
	}

	void SetValue(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(Program, name.c_str()), (int)value);
	}

	void SetValue(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(Program, name.c_str()), value);
	}

	void SetValue(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(Program, name.c_str()), value);
	}

	void SetValue(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(Program, name.c_str()), 1, &value[0]);
	}
	void SetValue(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(Program, name.c_str()), x, y);
	}

	void SetValue(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(Program, name.c_str()), 1, &value[0]);
	}
	
	void SetValue(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(Program, name.c_str()), x, y, z);
	}

	void SetValue(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(Program, name.c_str()), 1, &value[0]);
	}
	
	void SetValue(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(Program, name.c_str()), x, y, z, w);
	}

	void SetValue(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void SetValue(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void SetValue(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	static void CheckCompileErrors(u32 shader, const std::string& type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- \n";
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- \n";
			}
		}
	}
};