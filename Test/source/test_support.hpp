#pragma once

#include "camera.hpp"
#include "graphics/graphicsdevice.hpp"
#include "graphics/light.hpp"
#include "graphics/material.hpp"
#include "graphics/textures.hpp"
#include "scenes/scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// A scene with a single cube at a known position.
class TestScene : public Scene
{
public:
	TestScene(GraphicsDevice& device, const glm::vec3& objectPosition);

	void Cleanup() override;
	void InternalUpdate(f32 /*deltaTime*/, const Camera& /*camera*/) override;

	void AddLight(LightType type, const glm::vec3& position, const glm::vec3& color, f32 radius);

private:
	GraphicsDevice& _device;
	glm::vec3 _objectPosition;

	Texture* _diffuse{};
	Texture* _normal{};
	Texture* _specular{};
	Material* _material{};
};

// Reads a colour texture back as RGBA floats, bottom row first.
std::vector<f32> ReadTexture(const Texture& texture, s32 width, s32 height);

// Reads the presented image from the default framebuffer as RGBA floats.
std::vector<f32> ReadDefaultFramebuffer(s32 width, s32 height);
