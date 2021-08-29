#pragma once
#include <emptyspace/types.hpp>
#include <emptyspace/io/file.hpp>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <stdexcept>

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
    [[nodiscard]] u32 GetId() const;

    Program(
        const std::string_view label,
        const std::string_view vertexShaderFilePath,
        const std::string_view fragmentShaderFilePath);
    ~Program();

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
        
    void Bind() const;
    
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

    static void ValidateProgram(const u32 shader, const std::string_view filename);

    u32 _pipeline{};
    u32 _vertexShader{};
    u32 _fragmentShader{};
};
