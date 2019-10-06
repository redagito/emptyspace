#pragma once

#include <glm/mat4x4.hpp>

class Material;

enum class Shape
{
	Cube = 0,
	Quad = 1,
	CubeInstanced = 2,
	Ship = 3
};

struct SceneObject
{
	glm::mat4 ModelViewProjection;
	glm::mat4 ModelViewProjectionPrevious;
	Shape ObjectShape;
	Material* ObjectMaterial;
	bool ExcludeFromMotionBlur;

	explicit SceneObject(const Shape shape = Shape::Cube, Material* material = nullptr, const bool excludeFromMotionBlur = false)
		: ModelViewProjection{}, ModelViewProjectionPrevious{}, ObjectShape{ shape }, ObjectMaterial{ material }, ExcludeFromMotionBlur{ excludeFromMotionBlur }
	{
	}
};