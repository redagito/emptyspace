#pragma once
#include <emptyspace/types.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>

#include <vector>
#include <stdexcept>
#include <array>
#include <filesystem>
#include <sstream>

u32 CreateTexture2D(const u32 internalFormat, const u32 format, const s32 width, const s32 height, void* data = nullptr, const u32 filter = GL_LINEAR, const u32 repeat = GL_REPEAT)
{
	u32 name = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glTextureStorage2D(name, 1, internalFormat, width, height);

	glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, filter);
	glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, filter);
	glTextureParameteri(name, GL_TEXTURE_WRAP_S, repeat);
	glTextureParameteri(name, GL_TEXTURE_WRAP_T, repeat);

	if (data)
	{
		glTextureSubImage2D(name, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	}

	return name;
}

template <typename T = nullptr_t>
u32 CreateTextureCube(const u32 internalFormat, const u32 format, const s32 width, const s32 height, std::array<T*, 6> const& data)
{
	u32 name = 0;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
	glTextureStorage2D(name, 1, internalFormat, width, height);

	for (s32 i = 0; i < 6; ++i)
	{
		if (data[i])
		{
			glTextureSubImage3D(name, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data[i]);
		}
	}

	return name;
}

using stb_comp_t = decltype(STBI_default);

GLuint CreateTexture2DFromfile(const std::string_view filepath, stb_comp_t comp = STBI_rgb_alpha)
{
	s32 width{};
	s32 height{};
	s32 components{};

	if (!std::filesystem::exists(filepath.data()))
	{
		std::ostringstream message;
		message << "file " << filepath.data() << " does not exist.";
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

	const auto name = CreateTexture2D(internalFormat, format, width, height, data);
	stbi_image_free(data);
	return name;
}

GLuint CreateTextureCubeFromFile(const std::array<std::string_view, 6> & filepath, stb_comp_t comp = STBI_rgb_alpha)
{
	s32 width{};
	s32 height{};
	s32 components{};

	std::array<stbi_uc*, 6> faces{};

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

	for (auto i = 0; i < 6; i++)
	{
		faces[i] = stbi_load(filepath[i].data(), &width, &height, &components, comp);
	}

	const auto name = CreateTextureCube(in, ex, width, height, faces);

	for (auto face : faces)
	{
		stbi_image_free(face);
	}
	return name;
}

u32 CreateFramebuffer(const std::vector<GLuint>& colorAttachments, u32 depthAttachment = GL_NONE)
{
	u32 name = 0;
	glCreateFramebuffers(1, &name);

	for (auto i = 0; i < colorAttachments.size(); i++)
	{
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT0 + i, colorAttachments[i], 0);
	}

	std::array<u32, 32> drawBuffers{};
	for (u32 i = 0; i < colorAttachments.size(); i++)
	{
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	glNamedFramebufferDrawBuffers(name, u32(colorAttachments.size()), drawBuffers.data());

	if (depthAttachment != GL_NONE)
	{
		glNamedFramebufferTexture(name, GL_DEPTH_ATTACHMENT, depthAttachment, 0);
	}

	if (glCheckNamedFramebufferStatus(name, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("incomplete framebuffer");
	}
	return name;
}