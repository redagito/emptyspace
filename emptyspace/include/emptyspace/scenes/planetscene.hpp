#pragma once

#include <emptyspace/scenes/scene.hpp>

class PlanetScene : public Scene
{
public:
	PlanetScene()
	{

	}

	~PlanetScene() override
	{

	}

	void Initialize() override
	{
		InitializeTextures();
	}

protected:

	void InternalDraw(f32 deltaTime) override
	{
	}

	void InternalUpdate(f32 deltaTime) override
	{
	}

private:

	void InitializeTextures()
	{

	}
};