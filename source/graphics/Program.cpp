#include <emptyspace/graphics/program.hpp>

#include <iostream>
#include <sstream>

[[nodiscard]] u32 Program::GetId() const
{
    return _pipeline;
}

Program::Program(
    const std::string_view label,
    const std::string_view vertexShaderFilePath,
    const std::string_view fragmentShaderFilePath)
{
    auto const vertexShaderSource = ReadTextFile(vertexShaderFilePath);
    auto const fragmentShaderSource = ReadTextFile(fragmentShaderFilePath);

    auto const vertexShaderData = vertexShaderSource.data();
    auto const fragmentShaderData = fragmentShaderSource.data();

    _vertexShader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShaderData);
    _fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShaderData);
#ifdef _DEBUG
    glObjectLabel(GL_PROGRAM, _vertexShader, static_cast<GLsizei>(vertexShaderFilePath.length()), vertexShaderFilePath.data());
    glObjectLabel(GL_PROGRAM, _fragmentShader, static_cast<GLsizei>(fragmentShaderFilePath.length()), fragmentShaderFilePath.data());
#endif
    ValidateProgram(_vertexShader, vertexShaderFilePath);
    ValidateProgram(_fragmentShader, fragmentShaderFilePath);

    glCreateProgramPipelines(1, &_pipeline);
    glUseProgramStages(_pipeline, GL_VERTEX_SHADER_BIT, _vertexShader);
    glUseProgramStages(_pipeline, GL_FRAGMENT_SHADER_BIT, _fragmentShader);
#ifdef _DEBUG
    glObjectLabel(GL_PROGRAM_PIPELINE, _pipeline, static_cast<GLsizei>(label.length()), label.data());
#endif
}

Program::~Program()
{
    glDeleteProgramPipelines(1, &_pipeline);
    glDeleteProgram(_vertexShader);
    glDeleteProgram(_fragmentShader);
}

void Program::Bind() const
{
    glBindProgramPipeline(_pipeline);
}

void Program::ValidateProgram(const u32 shader, const std::string_view filename)
{
    auto compiled = 0;
    glProgramParameteri(shader, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glGetProgramiv(shader, GL_LINK_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        std::array<char, 1024> compilerLog{};
        glGetProgramInfoLog(shader, static_cast<u32>(compilerLog.size()), nullptr, compilerLog.data());
        glDeleteShader(shader);

        std::ostringstream message;
        message << "SHADER: " << filename << " contains error(s):\n\n" << compilerLog.data() << '\n';
        std::cout << message.str();
        __debugbreak();
    }
}