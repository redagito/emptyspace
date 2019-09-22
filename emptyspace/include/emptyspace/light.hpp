#pragma once
#include <glm/vec3.hpp>

struct Light
{
	glm::vec4 Position;
	glm::vec4 Color;
	glm::vec4 Attenuation;

	Light(glm::vec3 position, glm::vec3 color, float attenuation)
		: Position(position, 0.0f), Color(color, 0.0f), Attenuation(attenuation, 0.0f, 0.0f, 0.0f)
	{
	}
};
