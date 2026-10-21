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
        for (auto* object : _objects)
        {
            delete object;
        }
        delete RootNode;
    }

    virtual void Cleanup() = 0;

    virtual void Initialize();

    inline void Update(const f32 deltaTime, const Camera& camera)
    {
        InternalUpdate(deltaTime, camera);
    }

    // Binds the per instance data the geometry shader reads. Scenes without
    // instanced geometry do not override this.
    virtual void BindInstanceBuffer()
    {
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
    virtual void InternalUpdate(f32 /*deltaTime*/, const Camera& /*camera*/)
    {
    }

    std::vector<Light> _lights;
    std::vector<Material*> _materials;
    std::vector<SceneObject*> _objects;

    SceneNode* RootNode;
};

inline void Scene::Initialize()
{
}
