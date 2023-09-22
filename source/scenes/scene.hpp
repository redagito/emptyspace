#pragma once

#include <vector>

#include "types.hpp"
#include "camera.hpp"
#include "graphics/light.hpp"
#include "scenes/scenenode.hpp"

class Material;
struct SceneObject;

class Scene
{
public:
    Scene()
    {
        RootNode = new SceneNode(nullptr);
    }

    virtual ~Scene()
    {

    }

    virtual void Cleanup() = 0;

    inline void Draw(f32 deltaTime)
    {
        InternalDraw(deltaTime);
    }

    virtual void Initialize();

    inline void Update(const f32 deltaTime, const Camera& camera)
    {
        InternalUpdate(deltaTime, camera);
    }

    std::vector<Light>& Lights()
    {
        return _lights;
    }
    
    std::vector<SceneObject*>& Objects()
    {
        return _objects;
    }

protected:
    virtual void InternalDraw(f32 /*deltaTime*/)
    {
    }

    virtual void InternalUpdate(f32 /*deltaTime*/, const Camera& /*camera*/)
    {
    }

    std::vector<Light> _lights;
    std::vector<Material*> _materials;
    std::vector<SceneObject*> _objects;

    SceneNode* RootNode;
private:

};

inline void Scene::Initialize()
{
}
