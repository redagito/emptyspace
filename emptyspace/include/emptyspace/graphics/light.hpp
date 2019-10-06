#pragma once
#include <emptyspace/types.hpp>
#include <glm/vec3.hpp>

struct Light
{
	s32 Type;
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Direction;
	glm::vec3 Attenuation;
	glm::vec2 CutOff;

	explicit Light(s32 type, glm::vec3 position, glm::vec3 color, glm::vec3 attenuation, glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2 cutOff = glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f))))
		: Type{ type }, Position{ position }, Color{ color }, Direction{ direction }, Attenuation{ attenuation }, CutOff{ cutOff }
	{
	}
};
