#pragma once

#include <emptyspace/types.hpp>
#include <emptyspace/camera.hpp>
#include <emptyspace/graphics/light.hpp>

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

	virtual void Initialize() = 0;

	inline void Update(f32 deltaTime, const Camera& camera)
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
	virtual void InternalDraw(f32 deltaTime)
	{
	}

	virtual void InternalUpdate(f32 deltaTime, const Camera& camera)
	{
	}

	std::vector<Light> _lights;
	std::vector<Material*> _materials;
	std::vector<SceneObject*> _objects;

	SceneNode* RootNode;
private:

};