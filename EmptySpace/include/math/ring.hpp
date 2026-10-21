#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cstddef>

// The angle of an item on a ring around the origin, in radians.
inline f32 RingAngle(const std::size_t index, const std::size_t count)
{
	return glm::radians(static_cast<f32>(index) / static_cast<f32>(count) * 360.0f);
}

// The horizontal position of an item on a ring around the origin.
inline glm::vec2 RingPosition(const std::size_t index, const std::size_t count, const f32 radius)
{
	const auto angle = RingAngle(index, count);
	return glm::vec2(sin(angle) * radius, cos(angle) * radius);
}
