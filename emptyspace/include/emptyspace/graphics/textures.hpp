#pragma once

#include <emptyspace/types.hpp>

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <filesystem>
#include <sstream>

inline u32 CreateTexture2D(const u32 internalFormat, const u32 format, const s32 width, const s32 height,
                           void* data = nullptr, const u32 filter = GL_LINEAR, const u32 repeat = GL_REPEAT)
{
	u32 id = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	glTextureStorage2D(id, 1, internalFormat, width, height);

	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, filter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, filter);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, repeat);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, repeat);
	glTextureParameteri(id, GL_TEXTURE_WRAP_R, repeat);

	if (data)
	{
		glTextureSubImage2D(id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	}

	return id;
}

using stb_comp_t = decltype(STBI_default);
inline u32 CreateTexture2DFromFile(const std::string_view filepath, stb_comp_t comp = STBI_rgb_alpha)
{
	int x, y, c;
	if (!std::filesystem::exists(filepath.data()))
	{
		std::ostringstream message;
		message << "file " << filepath.data() << " does not exist.";
		throw std::runtime_error(message.str());
	}

	const auto data = stbi_load(filepath.data(), &x, &y, &c, comp);

	auto const [in, ex] = [comp]()
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

	const auto name = CreateTexture2D(in, ex, x, y, data);
	stbi_image_free(data);
	return name;
}

template <typename T = nullptr_t>
u32 CreateTextureCube(const u32 internalFormat, const u32 format, const s32 width, const s32 height,
                      std::array<T*, 6> const& data)
{
	GLuint tex = 0;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex);
	glTextureStorage2D(tex, 1, internalFormat, width, height);

	for (auto i = 0; i < 6; ++i)
	{
		if (data[i])
		{
			glTextureSubImage3D(tex, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data[i]);
		}
	}

	return tex;
}

inline u32 CreateTextureCubeFromFile(const std::array<std::string_view, 6>& filepaths, stb_comp_t comp = STBI_rgb_alpha)
{
	int x, y, c;
	std::array<stbi_uc*, 6> faces{};

	auto const [in, ex] = [comp]() {
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
		faces[i] = stbi_load(filepaths[i].data(), &x, &y, &c, comp);
	}

	const auto name = CreateTextureCube(in, ex, x, y, faces);

	for (auto face : faces)
	{
		stbi_image_free(face);
	}
	return name;
}