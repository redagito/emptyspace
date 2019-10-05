#pragma once

#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/program.hpp>
#include <emptyspace/graphics/textures.hpp>

#include <glad/glad.h>

class GraphicsDevice final
{
public:
	GraphicsDevice()
	{

	}

	~GraphicsDevice()
	{

	}

	void DestroyTexture(u32 texture)
	{
		glDeleteTextures(1, &texture);
	}
private:

};