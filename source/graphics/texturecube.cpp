#include "graphics/texturecube.hpp"

#include <stdexcept>
#include <cstring>

TextureCube* TextureCube::FromFiles(const std::array<std::string_view, 6>& filePaths, u32 comp)
{
    s32 width{};
    s32 height{};
    s32 components{};

    std::array<stbi_uc*, 6> faces{};

    auto const [internalFormat, format] = [comp]()
    {
        switch (comp)
        {
        case STBI_rgb_alpha: return std::make_pair(GL_RGBA8, GL_RGBA);
        case STBI_rgb: return std::make_pair(GL_RGB8, GL_RGB);
        case STBI_grey: return std::make_pair(GL_R8, GL_RED);
        case STBI_grey_alpha: return std::make_pair(GL_RG8, GL_RG);
        default: throw std::runtime_error("invalid format");
        }
    }();

    for (auto i = 0; i < 6; i++)
    {
        faces[i] = stbi_load(filePaths[i].data(), &width, &height, &components, comp);
    }

    const auto textureCube = new TextureCube(internalFormat, format, width, height, faces);

    for (auto face : faces)
    {
        stbi_image_free(face);
    }
    return textureCube;
}

TextureCube::TextureCube(const u32 internalFormat, const u32 format, const s32 width, const s32 height, std::array<stbi_uc*, 6> const& data)
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_id);
    glTextureStorage2D(_id, 1, internalFormat, width, height);

    for (auto i = 0; i < 6; ++i)
    {
        if (data[i])
        {
            glTextureSubImage3D(_id, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data[i]);
        }
    }

    char label[64];
    snprintf(label, sizeof(label), "TC_%dx%d %d", width, height, format);
    glObjectLabel(GL_TEXTURE, _id, static_cast<GLsizei>(strlen(label)), label);
}

TextureCube::~TextureCube()
{
    glDeleteTextures(1, &_id);
}

u32 TextureCube::Id() const
{
    return _id;
}

void TextureCube::Bind(const u32 textureUnit) const
{
    glBindTextureUnit(textureUnit, _id);
}