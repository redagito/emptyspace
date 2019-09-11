#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct VertexPositionColorNormalUv
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Normal;
	glm::vec2 Uv;

	VertexPositionColorNormalUv(glm::vec3 const& position, glm::vec3 const& color, glm::vec3 const& normal, glm::vec2 const& uv)
		: Position(position), Color(color), Normal(normal), Uv(uv)
	{
	}
};