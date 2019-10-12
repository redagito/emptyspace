#pragma once

#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/program.hpp>
#include <emptyspace/graphics/textures.hpp>

#include <glad/glad.h>

class GraphicsDevice final
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	Texture* CreateTexture(const u32 internalFormat, const u32 format, const s32 width, const s32 height, void* data = nullptr, const u32 filter = GL_LINEAR, const u32 repeat = GL_REPEAT);
private:

};