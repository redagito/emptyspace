#pragma once

#include <emptyspace/types.hpp>

class Scene
{
public:
	Scene()
	{

	}

	virtual ~Scene()
	{

	}

	inline void Draw(f32 deltaTime)
	{
		InternalDraw(deltaTime);
	}

	virtual void Initialize() = 0;

	inline void Update(f32 deltaTime)
	{
		InternalUpdate(deltaTime);
	}
protected:
	virtual void InternalDraw(f32 deltaTime)
	{
	}

	virtual void InternalUpdate(f32 deltaTime) 
	{
	}
};