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

// A scene with a single object at a known place.
class TestScene : public Scene
{
public:
	// The normal map defaults to a flat one, the specular map to a black one.
	// Pass four bytes to use another.
	TestScene(GraphicsDevice& device, Shape shape, const glm::mat4& model,
		const u8* normalMap = nullptr, const u8* specularMap = nullptr);

	void Cleanup() override;
	void InternalUpdate(f32 /*deltaTime*/, const Camera& /*camera*/) override;

	void AddLight(LightType type, const glm::vec3& position, const glm::vec3& color, f32 radius,
		const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 1.0f));

private:
	GraphicsDevice& _device;
	glm::mat4 _model;

	Texture* _diffuse{};
	Texture* _normal{};
	Texture* _specular{};
	Material* _material{};
};

// Reads a colour texture back as RGBA floats, bottom row first.
std::vector<f32> ReadTexture(const Texture& texture, s32 width, s32 height);

// Reads the presented image from the default framebuffer as RGBA floats.
std::vector<f32> ReadDefaultFramebuffer(s32 width, s32 height);
