#pragma once

#include "scenes/scene.hpp"

class PlanetScene : public Scene
{
public:
    PlanetScene()
    {

    }

    ~PlanetScene() override
    {

    }

    void Cleanup() override
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

    void InternalUpdate(f32 deltaTime, const Camera& camera) override
    {
    }

private:

    void InitializeTextures()
    {

    }
};