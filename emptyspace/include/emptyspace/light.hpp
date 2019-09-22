#pragma once
#include <glm/vec3.hpp>

struct Light
{
	glm::vec3 Position;
	glm::vec3 Color;
	float Attenuation;

	Light(glm::vec3 position, glm::vec3 color, float attenuation)
		: Position(position), Color(color), Attenuation(attenuation)
	{
	}
};
