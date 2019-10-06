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

	void ClearFramebuffer(u32 framebuffer, glm::vec3 clearColor, bool clearDepth = true, f32 clearDepthValue = 1.0f) const
	{
		glClearNamedFramebufferfv(framebuffer, GL_COLOR, 0, glm::value_ptr(clearColor));
		if (clearDepth)
		{
			glClearNamedFramebufferfv(framebuffer, GL_DEPTH, 0, &clearDepthValue);
		}
	}

	void ClearFramebuffer(u32 framebuffer, s32 components, glm::vec3 clearColor, bool clearDepth = true, f32 clearDepthValue = 1.0f) const
	{
		for (s32 component = 0; component < components; component++)
		{
			glClearNamedFramebufferfv(framebuffer, GL_COLOR, component, glm::value_ptr(clearColor));
		}
		if (clearDepth)
		{
			glClearNamedFramebufferfv(framebuffer, GL_DEPTH, 0, &clearDepthValue);
		}
	}

	void DestroyTexture(u32 texture) const
	{
		glDeleteTextures(1, &texture);
	}
private:

};