#pragma once

#include <emptyspace/types.hpp>

struct Light;
class Material;
struct SceneObject;

class Scene
{
public:
	Scene()
	{

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

	inline void Update(f32 deltaTime)
	{
		InternalUpdate(deltaTime);
	}

	std::vector<SceneObject*>& Objects()
	{
		return _objects;
	}

protected:
	virtual void InternalDraw(f32 deltaTime)
	{
	}

	virtual void InternalUpdate(f32 deltaTime) 
	{
	}

private:
	std::vector<Light*> _lights;
	std::vector<Material*> _materials;
	std::vector<SceneObject*> _objects;
};