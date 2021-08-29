#pragma once

#include <emptyspace/types.hpp>

#include <glad/glad.h>
#include <stb_image.h>

#include <array>
#include <string_view>
#include <vector>

class Framebuffer;
class Texture;
class TextureCube;
class Program;

class GraphicsDevice final
{
public:
    GraphicsDevice();
    ~GraphicsDevice();

    Texture* CreateTexture(
        const u32 internalFormat,
        const u32 format,
        const s32 width,
        const s32 height,
        void* data = nullptr,
        const u32 filter = GL_LINEAR,
        const u32 wrap = GL_REPEAT);

    Texture* CreateTextureFromFile(
        const std::string_view filePath,
        const u32 comp = STBI_rgb_alpha);

    TextureCube* CreateTextureCubeFromFiles(
        const std::array<std::string_view, 6>& filePaths,
        const u32 comp = STBI_rgb_alpha);

    Framebuffer* CreateFramebuffer(
        const std::string_view label,
        const std::vector<Texture*>& colorAttachments,
        const Texture* depthAttachment = nullptr);

    Program* CreateProgramFromFiles(
        const std::string_view label,
        const std::string_view vertexShaderFilePath,
        const std::string_view fragmentShaderFilePath);
private:

};
