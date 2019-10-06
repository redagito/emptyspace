#pragma once

#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/graphicsdevice.hpp>
#include <emptyspace/graphics/program.hpp>
#include <emptyspace/graphics/textures.hpp>
#include <emptyspace/scenes/scene.hpp>

#include <GLFW/glfw3.h>

class SpaceScene : public Scene
{
public:
	SpaceScene(GraphicsDevice& graphicsDevice)
		: _graphicsDevice{ graphicsDevice }
	{

	}

	~SpaceScene() override
	{
		_graphicsDevice.DestroyTexture(_textureCubeDiffuse);
		_graphicsDevice.DestroyTexture(_textureCubeSpecular);
		_graphicsDevice.DestroyTexture(_textureCubeNormal);
		_graphicsDevice.DestroyTexture(_textureSkybox);

		delete _bufferAsteroids;
	}

	void Initialize() override
	{
		InitializeTextures();

		
	}


protected:
	void InitializeTextures()
	{
		_textureCubeDiffuse = CreateTexture2DFromfile("./res/textures/T_Default_D.png", STBI_rgb);
		_textureCubeSpecular = CreateTexture2DFromfile("./res/textures/T_Default_S.png", STBI_grey);
		_textureCubeNormal = CreateTexture2DFromfile("./res/textures/T_Default_N.png", STBI_rgb);
		_textureSkybox = CreateTextureCubeFromFiles({
			"./res/textures/TC_SkySpace_Xn.png",
			"./res/textures/TC_SkySpace_Xp.png",
			"./res/textures/TC_SkySpace_Yn.png",
			"./res/textures/TC_SkySpace_Yp.png",
			"./res/textures/TC_SkySpace_Zn.png",
			"./res/textures/TC_SkySpace_Zp.png"
			});

		auto const asteroidInstances = CreateAsteroidInstances(5000);

		_bufferAsteroids = new Buffer(asteroidInstances);

		auto lights = CreateRandomLights(100);
		lights.emplace_back(1, glm::vec3(0), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0032f, 0.09f, 32.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void InternalDraw(f32 deltaTime) override
	{
	}

	void InternalUpdate(f32 deltaTime) override
	{
	}

private:
	// shamelessly stolen from the learnopengl tutorial
	std::vector<glm::mat4> CreateAsteroidInstances(const u32 instanceCount) const
	{
		std::vector<glm::mat4> modelMatrices;

		srand(static_cast<int>(glfwGetTime()));
		const auto radius = 200.0f;
		const auto offset = 100.5f;
		for (u32 i = 0; i < instanceCount; i++)
		{
			auto model = glm::mat4(1.0f);
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			const auto angle = static_cast<float>(i) / static_cast<float>(instanceCount) * 360.0f;
			auto displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto x = sin(angle) * radius + displacement;
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto z = cos(angle) * radius + displacement;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: Scale between 0.05 and 0.25f
			const auto scale = rand() % 60 / 100.0f + 2.05f;
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			const auto rotAngle = f32(rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			modelMatrices.push_back(model);
		}

		return modelMatrices;
	}

	std::vector<Light> CreateRandomLights(const u32 instanceCount) const 
	{
		std::vector<Light> lights;

		srand(static_cast<int>(glfwGetTime()));
		const auto radius = 200.0f;
		const auto offset = 100.5f;
		for (u32 i = 0; i < instanceCount; i++)
		{
			auto model = glm::mat4(1.0f);
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			const auto angle = static_cast<float>(i) / static_cast<float>(instanceCount) * 360.0f;
			auto displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto x = sin(angle) * radius + displacement;
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
			displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

			const auto z = cos(angle) * radius + displacement;
			const auto position = glm::vec3(x, y, z);

			const auto color = glm::vec3((rand() % 256) / 256.0f, (rand() % 256) / 256.0f, (rand() % 256) / 256.0f);

			// 2. scale: Scale between 0.05 and 0.25f
			const auto scale = rand() % 60;// / 100.0f + 2.05f;
			const auto attenuation = glm::vec3(scale);

			// 4. now add to list of matrices
			lights.emplace_back(0, position, color, glm::vec3(0.032f, 0.09f, attenuation.r));
		}

		return lights;
	}

	std::vector<Light> CreateLights() const
	{
		std::vector<Light> lights;
		lights.emplace_back(0, glm::vec3(-80, 1, +80), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.032f, 0.09f, 60.0f));
		lights.emplace_back(0, glm::vec3(-80, 1, -80), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.032f, 0.09f, 60.0f));
		lights.emplace_back(0, glm::vec3(+80, 1, -80), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.032f, 0.09f, 60.0f));
		lights.emplace_back(0, glm::vec3(+80, 1, +80), glm::vec3(1.2f, 0.3f, 1.1f), glm::vec3(0.032f, 0.09f, 60.0f));

		return lights;
	}

	
	GraphicsDevice& _graphicsDevice;
	u32 _textureCubeDiffuse{};
	u32 _textureCubeSpecular{};
	u32 _textureCubeNormal{};
	u32 _textureSkybox{};

	Buffer* _bufferAsteroids;
	std::vector<Scene*> _scenes;

};