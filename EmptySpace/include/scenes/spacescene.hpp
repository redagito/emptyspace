#pragma once

#include "graphics/geometry.hpp"
#include "graphics/graphicsdevice.hpp"
#include "graphics/material.hpp"
#include "graphics/program.hpp"
#include "graphics/textures.hpp"
#include "math/ring.hpp"
#include "scenes/scene.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtx/quaternion.hpp>

class SpaceScene : public Scene
{
public:
	// The rate the cubes orbit the origin at, in radians per second.
	static constexpr f32 OrbitRate = 0.35f;

	// The rate the cubes spin on their own axis at, in radians per second.
	static constexpr f32 SpinRate = 0.6f;

	SpaceScene(GraphicsDevice& graphicsDevice)
		: _graphicsDevice{ graphicsDevice }
	{
	}

	~SpaceScene() override
	{
		Cleanup();
	}

	void Cleanup() override
	{
		delete _textureCubeDiffuse;
		delete _textureCubeSpecular;
		delete _textureCubeNormal;

		delete _bufferAsteroids;
		delete _defaultMaterial;
	}

	void Initialize() override
	{
		InitializeTextures();
		InitializeLights();

		_objects.push_back(new SceneObject(Shape::Quad, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::Cube, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::Ship, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::Cube, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::Cube, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::Cube, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::Cube, _defaultMaterial));
		_objects.push_back(new SceneObject(Shape::CubeInstanced, _defaultMaterial));
	}

	void BindInstanceBuffer() override
	{
		_bufferAsteroids->BindAsStorageBuffer(0);
	}

	protected:
	void InitializeLights()
	{
		auto lights = CreateRandomLights(50);
		lights.emplace_back(LightType::SpotLight,
			glm::vec3(0.0f, 16.0f, 0.0f),                             // the position of the light
			glm::vec3(1.0f, 1.0f, 1.0f),                               // the colour of the light
			400.0f,                                                     // the radius it fades out over
			glm::vec3(0.0f, -1.0f, 0.0f),                              // the direction it points in
			glm::vec2(glm::cos(glm::radians(30.0f)), glm::cos(glm::radians(35.0f))));  // its cone

		for (auto& light : lights)
		{
			_lights.push_back(light);
		}
	}

	void InitializeTextures()
	{
		_textureCubeDiffuse = _graphicsDevice.CreateTextureFromFile("data/textures/T_PlasticMesh_D.jpg", STBI_rgb);
		// The specular map is greyscale but it is sampled as a colour, so it
		// has to keep its value in every channel.
		_textureCubeSpecular = _graphicsDevice.CreateTextureFromFile("data/textures/T_PlasticMesh_S.jpg", STBI_rgb);
		_textureCubeNormal = _graphicsDevice.CreateTextureFromFile("data/textures/T_PlasticMesh_N.jpg", STBI_rgb);

		_defaultMaterial = new Material(_textureCubeDiffuse, _textureCubeNormal, _textureCubeSpecular);

		auto const asteroidInstances = CreateAsteroidInstances(5000);

		_bufferAsteroids = new Buffer(asteroidInstances);
	}

	void InternalUpdate(const f32 deltaTime, const Camera& camera) override
	{
		// The cubes orbit and spin at a fixed rate, whatever the frame rate is.
		_orbitProgression += deltaTime * OrbitRate;
		_spinProgression += deltaTime * SpinRate;

		auto const orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f);

		_objects[0]->ModelViewProjection = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
		_objects[1]->ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter) * glm::rotate(glm::mat4(1.0f), _spinProgression, glm::vec3(0.0f, 1.0f, 0.0f));

		// The ship is drawn with the orientation of the camera, a little behind
		// it and off to the side.
		const auto shipOffset = camera.Orientation * glm::vec3(0.25f, -0.5f, 0.25f);
		auto shipModel = glm::translate(glm::mat4(1.0f), camera.Position + shipOffset);
		shipModel *= glm::mat4_cast(camera.Orientation);
		_objects[2]->ModelViewProjection = shipModel;

		const auto objectCount = _objects.size();
		for (std::size_t i = 3; i < objectCount; i++)
		{
			const auto orbitAmount = _orbitProgression + static_cast<f32>(i) * 90.0f * glm::pi<f32>() / 180.0f;
			const auto orbitPosition = OrbitAxis(orbitAmount, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(2.0f, 0.0f, 2.0f));
			_objects[i]->ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter + orbitPosition) * glm::rotate(glm::mat4(1.0f), _spinProgression + orbitAmount, glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}

private:
	// shamelessly stolen from the learnopengl tutorial
	[[nodiscard]] std::vector<glm::mat4> CreateAsteroidInstances(const u32 instanceCount) const
	{
		std::vector<glm::mat4> modelMatrices;

		srand(static_cast<int>(glfwGetTime()));
		const auto radius = 200.0f;
		const auto offset = 100.5f;
		for (u32 i = 0; i < instanceCount; i++)
		{
			auto model = glm::mat4(1.0f);
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			const auto ring = RingPosition(i, instanceCount, radius);
			auto displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto x = ring.x + displacement;
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto z = ring.y + displacement;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: Scale between 0.05 and 0.25f
			const auto scale = rand() % 60 / 100.0f + 2.05f;
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			const auto rotAngle = static_cast<f32>(rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			modelMatrices.push_back(model);
		}

		return modelMatrices;
	}

	[[nodiscard]] std::vector<Light> CreateRandomLights(const u32 lightCount) const
	{
		std::vector<Light> lights;

		srand(static_cast<int>(glfwGetTime()));
		const auto radius = 200.0f;
		const auto offset = 100.5f;
		for (u32 i = 0; i < lightCount; i++)
		{
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			const auto ring = RingPosition(i, lightCount, radius);
			auto displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto x = ring.x + displacement;
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto z = ring.y + displacement;
			const auto position = glm::vec3(x, y, z);

			const auto color = glm::vec3((rand() % 256) / 256.0f, (rand() % 256) / 256.0f, (rand() % 256) / 256.0f);

			// The radius the light fades out over.
			const auto attenuation = static_cast<f32>(rand() % 60 + 1);

			// 4. now add to list of matrices
			lights.emplace_back(LightType::PointLight, position, color, attenuation);
		}

		return lights;
	}

	static glm::vec3 OrbitAxis(const f32 angle, const glm::vec3& axis, const glm::vec3& spread)
	{
		return glm::angleAxis(angle, axis) * spread;
	}

	Material* _defaultMaterial{};

	GraphicsDevice& _graphicsDevice;
	Texture* _textureCubeDiffuse{};
	Texture* _textureCubeSpecular{};
	Texture* _textureCubeNormal{};

	Buffer* _bufferAsteroids{};

	f32 _orbitProgression{ 0.0f };
	f32 _spinProgression{ 0.0f };
};
