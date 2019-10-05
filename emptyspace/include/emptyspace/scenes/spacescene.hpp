#pragma once

#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/graphicsdevice.hpp>
#include <emptyspace/graphics/program.hpp>
#include <emptyspace/graphics/textures.hpp>
#include <emptyspace/scenes/scene.hpp>

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
	}

	void InternalDraw(f32 deltaTime) override
	{
	}

	void InternalUpdate(f32 deltaTime) override
	{
	}

private:
	GraphicsDevice& _graphicsDevice;
	u32 _textureCubeDiffuse{};
	u32 _textureCubeSpecular{};
	u32 _textureCubeNormal{};
	u32 _textureSkybox{};

};