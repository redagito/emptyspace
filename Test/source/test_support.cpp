#include "test_support.hpp"

#include "graphics/geometry.hpp"

#include <glad/glad.h>
#include <glm/gtx/quaternion.hpp>

TestScene::TestScene(GraphicsDevice& device, const Shape shape, const glm::mat4& model,
	const u8* normalMap, const u8* specularMap, const glm::vec3& emission)
	: _device{ device }
	, _model{ model }
{
	const u8 white[4] = { 255, 255, 255, 255 };
	const u8 flatNormal[4] = { 128, 128, 255, 255 };
	const u8 black[4] = { 0, 0, 0, 255 };

	_diffuse = _device.CreateTexture(GL_RGBA8, GL_RGBA, 1, 1, const_cast<u8*>(white), GL_NEAREST);
	_normal = _device.CreateTexture(GL_RGBA8, GL_RGBA, 1, 1, const_cast<u8*>(normalMap != nullptr ? normalMap : flatNormal), GL_NEAREST);
	_specular = _device.CreateTexture(GL_RGBA8, GL_RGBA, 1, 1, const_cast<u8*>(specularMap != nullptr ? specularMap : black), GL_NEAREST);
	_material = new Material(_diffuse, _normal, _specular, emission);

	_objects.push_back(new SceneObject(shape, _material));
}

void TestScene::Cleanup()
{
	delete _material;
	delete _diffuse;
	delete _normal;
	delete _specular;
}

void TestScene::InternalUpdate(f32 /*deltaTime*/, const Camera& /*camera*/)
{
	_objects[0]->ModelViewProjection = _model;
}

void TestScene::AddLight(const LightType type, const glm::vec3& position, const glm::vec3& color, const f32 radius,
	const glm::vec3& direction)
{
	_lights.emplace_back(type, position, color, radius, direction);
}

std::vector<f32> ReadTexture(const Texture& texture, const s32 width, const s32 height)
{
	std::vector<f32> data(static_cast<std::size_t>(width) * height * 4);

	glBindTexture(GL_TEXTURE_2D, texture.Id());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	return data;
}

std::vector<f32> ReadDefaultFramebuffer(const s32 width, const s32 height)
{
	std::vector<f32> data(static_cast<std::size_t>(width) * height * 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, data.data());

	return data;
}
