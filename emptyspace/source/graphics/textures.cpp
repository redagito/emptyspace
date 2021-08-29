#include <emptyspace/graphics/textures.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <sstream>

Texture* Texture::FromFile(const std::string_view filepath, const u32 component)
{
    s32 width{};
    s32 height{};
    s32 components{};

    if (!std::filesystem::exists(filepath.data()))
    {
        std::ostringstream message;
        message << "Texture: File " << filepath.data() << " does not exist.";
        throw std::runtime_error(message.str());
    }
    const auto data = stbi_load(filepath.data(), &width, &height, &components, component);

    auto const [internalFormat, format] = [component]()
    {
        switch (component)
        {
        case STBI_rgb_alpha: return std::make_pair(GL_RGBA8, GL_RGBA);
        case STBI_rgb: return std::make_pair(GL_RGB8, GL_RGB);
        case STBI_grey: return std::make_pair(GL_R8, GL_RED);
        case STBI_grey_alpha: return std::make_pair(GL_RG8, GL_RG);
        default: throw std::runtime_error("GL: Invalid format");
        }
    }();

    const auto texture = new Texture(internalFormat, format, width, height, data);
    stbi_image_free(data);
    return texture;
}

Texture::Texture(const u32 internalFormat, const u32 format, const s32 width, const s32 height, void* data, const u32 filter, const u32 wrap)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &_id);
    glTextureStorage2D(_id, 1, internalFormat, width, height);

    glTextureParameteri(_id, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(_id, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(_id, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(_id, GL_TEXTURE_WRAP_T, wrap);

    if (data)
    {
        glTextureSubImage2D(_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    }

    char label[64];
    sprintf_s(label, "T_%dx%d_%s_%s_%s_%s", width, height, FormatToString(internalFormat), FormatToString(format), FilterToString(filter), WrapToString(wrap));
    glObjectLabel(GL_TEXTURE, _id, static_cast<GLsizei>(strlen(label)), label);
}

Texture::~Texture()
{
    glDeleteTextures(1, &_id);
}

u32 Texture::Id() const
{
    return _id;
}

void Texture::Bind(const u32 textureUnit) const
{
    glBindTextureUnit(textureUnit, _id);
}

const char* Texture::FilterToString(const GLuint filter)
{
    if (filter == GL_LINEAR)
    {
        return "Linear";
    }
    if (filter == GL_LINEAR_MIPMAP_LINEAR)
    {
        return "LinearMipmapLinear";
    }
    if (filter == GL_LINEAR_MIPMAP_NEAREST)
    {
        return "LinearMipmapNearest";
    }
    if (filter == GL_NEAREST)
    {
        return "Nearest";
    }
    if (filter == GL_NEAREST_MIPMAP_LINEAR)
    {
        return "NearestMipmapLinear";
    }
    if (filter == GL_NEAREST_MIPMAP_NEAREST)
    {
        return "NearestMipmapNearest";
    }

    return "UnknownFilter";
}

const char* Texture::WrapToString(const GLuint wrap)
{
    if (wrap == GL_CLAMP_TO_BORDER)
    {
        return "ClampToBorder";
    }
    if (wrap == GL_CLAMP_TO_EDGE)
    {
        return "ClampToEdge";
    }
    if (wrap == GL_REPEAT)
    {
        return "Repeat";
    }
    if (wrap == GL_MIRRORED_REPEAT)
    {
        return "MirroredRepeat";
    }

    return "UnknownWrapMode";
}

const char* Texture::FormatToString(const GLuint format)
{
    switch(format)
    {
    case GL_RED: return "R";
    case GL_R8: return "R8";
    case GL_R8I: return "R8i";
    case GL_R8UI: return "R8ui";
    case GL_R8_SNORM: return "R8snorm";

    case GL_R16: return "R16";
    case GL_R16F: return "R16f";
    case GL_R16I: return "R16i";
    case GL_R16_SNORM: return "R16snorm";

    case GL_R32F: return "R32f";
    case GL_R32I: return "R32i";

    case GL_RG: return "Rg";
    case GL_RG8: return "Rg8";
    case GL_RG8I: return "Rg8i";
    case GL_RG8UI: return "Rg8ui";
    case GL_RG8_SNORM: return "Rg8snorm";

    case GL_RG16: return "Rg16";
    case GL_RG16F: return "Rg16f";
    case GL_RG16I: return "Rg16i";
    case GL_RG16UI: return "Rg16ui";
    case GL_RG16_SNORM: return "Rg16snorm";

    case GL_RG32F: return "Rg32f";
    case GL_RG32I: return "Rg32i";

    case GL_RGB: return "Rgb";
    case GL_RGB8: return "Rgb8";
    case GL_RGB8I: return "Rgb8i";
    case GL_RGB8UI: return "Rgb8ui";
    case GL_RGB8_SNORM: return "Rgb8snorm";

    case GL_RGB16: return "Rgb16";
    case GL_RGB16F: return "Rgb16f";
    case GL_RGB16I: return "Rgb16i";
    case GL_RGB16UI: return "Rgb16ui";
    case GL_RGB16_SNORM: return "Rgb16snorm";

    case GL_RGB32F: return "Rgb32f";
    case GL_RGB32I: return "Rgb32i";
    case GL_RGB32UI: return "Rgb32ui";

    case GL_RGBA: return "Rgba";
    case GL_RGBA8: return "Rgba8";
    case GL_RGBA8I: return "Rgba8i";
    case GL_RGBA8UI: return "Rgba8ui";
    case GL_RGBA8_SNORM: return "Rgba8snorm";

    case GL_RGBA16: return "Rgba16";
    case GL_RGBA16F: return "Rgba16f";
    case GL_RGBA16I: return "Rgba16i";
    case GL_RGBA16_SNORM: return "Rgba16snorm";

    case GL_R11F_G11F_B10F: return "R11G11B10";

    case GL_RGB10: return "Rgb10";
    case GL_RGB10_A2: return "Rgb10a2";
    case GL_RGB565: return "Rgb565";

    case GL_DEPTH: return "D";
    case GL_DEPTH24_STENCIL8: return "D24S8";
    case GL_DEPTH32F_STENCIL8: return "D32fS8";
    case GL_DEPTH_COMPONENT: return "D";
    case GL_DEPTH_COMPONENT16: return "D16";
    case GL_DEPTH_COMPONENT24: return "D24";
    case GL_DEPTH_COMPONENT32: return "D32";
    case GL_DEPTH_COMPONENT32F: return "D32f";

    default: return "UnknownFormat";
    }
}