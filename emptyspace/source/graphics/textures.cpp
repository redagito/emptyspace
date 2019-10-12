#include <emptyspace/graphics/textures.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <sstream>

Texture* Texture::FromFile(const std::string_view filepath, u32 comp)
{
	s32 width{};
	s32 height{};
	s32 components{};

	if (!std::filesystem::exists(filepath.data()))
	{
		std::ostringstream message;
		message << "TXTURE: File " << filepath.data() << " does not exist.";
		throw std::runtime_error(message.str());
	}
	const auto data = stbi_load(filepath.data(), &width, &height, &components, comp);

	auto const [internalFormat, format] = [comp]()
	{
		switch (comp)
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

Texture::Texture(const u32 internalFormat, const u32 format, const s32 width, const s32 height, void* data, const u32 filter, const u32 repeat)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &_id);
	glTextureStorage2D(_id, 1, internalFormat, width, height);

	glTextureParameteri(_id, GL_TEXTURE_MIN_FILTER, filter);
	glTextureParameteri(_id, GL_TEXTURE_MAG_FILTER, filter);
	glTextureParameteri(_id, GL_TEXTURE_WRAP_S, repeat);
	glTextureParameteri(_id, GL_TEXTURE_WRAP_T, repeat);

	if (data)
	{
		glTextureSubImage2D(_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	}

	char label[64];
	sprintf_s(label, "T-%dx%d %d", width, height, format);
	const std::string_view labelTexture(label);
	glObjectLabel(GL_TEXTURE, _id, labelTexture.length(), labelTexture.data());
}

Texture::~Texture()
{
	glDeleteTextures(1, &_id);
}

u32 Texture::Id() const
{
	return _id;
}