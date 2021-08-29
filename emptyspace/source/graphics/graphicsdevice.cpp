#include <emptyspace/graphics/graphicsdevice.hpp>
#include <emptyspace/graphics/textures.hpp>
#include <emptyspace/graphics/texturecube.hpp>
#include <emptyspace/graphics/framebuffer.hpp>
#include <emptyspace/graphics/program.hpp>

#include <sstream>
#include <iostream>

#if _DEBUG
void APIENTRY DebugCallback(
    const u32 source,
    const u32 type,
    const u32 id,
    const u32 severity,
    s32 length,
    const GLchar* message,
    const void* userParam)
{
    if (id == 131185)
    {
        return;
    }
    if (type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP)
    {
        return;
    }

    std::ostringstream str;
    str << "---------------------GL CALLBACK---------------------\n";
    str << "Message: " << message << '\n';
    str << "Source: ";
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        str << "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        str << "WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        str << "SHADER_COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        str << "THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        str << "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        str << "OTHER";
        break;
    default:
        str << "UNKNOWN SOURCE";
        break;
    }
    str << '\n';
    str << "Type: ";
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        str << "ERROR";
        __debugbreak();
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        str << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        str << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        str << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        str << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        str << "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        str << "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        str << "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        str << "OTHER";
        break;
    default:
        str << "UNKNOWN TYPE";
        break;
    }
    str << '\n';
    str << "Id: " << id << '\n';
    str << "Severity: ";
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        //str << "NOTIFICATION";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        str << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        str << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        str << "HIGH";
        break;
    default:
        str << "UNKNOWN SEVERITY";
        break;
    }
    str << '\n';
    str << "---------------------GL CALLBACK---------------------\n";

    std::clog << str.str();
}
#endif

GraphicsDevice::GraphicsDevice()
{
    std::clog << "GL: VENDOR = " << glGetString(GL_VENDOR) << '\n';
    std::clog << "GL: VERSION = " << glGetString(GL_VERSION) << '\n';
    std::clog << "GL: GLSL VERSION = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

#if _DEBUG
    if (glDebugMessageCallback)
    {
        std::clog << "GL: Registered OpenGL Debug Callback.\n";
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(DebugCallback, nullptr);
        u32 unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
    }
    else
    {
        std::cerr << "GL: glDebugMessageCallback not available.\n";
    }
#endif

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

GraphicsDevice::~GraphicsDevice()
{
}

Texture* GraphicsDevice::CreateTexture(
    const u32 internalFormat,
    const u32 format,
    const s32 width,
    const s32 height,
    void* data,
    const u32 filter,
    const u32 wrap)
{
    return new Texture(internalFormat, format, width, height, data, filter, wrap);
}

Texture* GraphicsDevice::CreateTextureFromFile(
    const std::string_view filePath,
    const u32 comp)
{
    return Texture::FromFile(filePath, comp);
}

TextureCube* GraphicsDevice::CreateTextureCubeFromFiles(
    const std::array<std::string_view, 6>& filePaths,
    const u32 comp)
{
    return TextureCube::FromFiles(filePaths, comp);
}

Framebuffer* GraphicsDevice::CreateFramebuffer(
    const std::string_view label,
    const std::vector<Texture*>& colorAttachments,
    const Texture* depthAttachment)
{
    return new Framebuffer(label, colorAttachments, depthAttachment);
}

Program* GraphicsDevice::CreateProgramFromFiles(
        const std::string_view label,
        const std::string_view vertexShaderFilePath,
        const std::string_view fragmentShaderFilePath)
{
    return new Program(label, vertexShaderFilePath, fragmentShaderFilePath);
}