#pragma once
#include <glm/vec3.hpp>

struct Light
{
	glm::vec3 Position;
	glm::vec3 Color;
	float Attenuation;
};
