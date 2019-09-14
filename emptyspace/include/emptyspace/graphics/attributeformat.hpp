#pragma once
#include "emptyspace/types.hpp"

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <utility>
#include <stdexcept>

struct AttributeFormat
{
	u32 Index;
	s32 Size;
	u32 Type;
	u32 RelativeOffset;
};

template<typename T>
constexpr std::pair<GLint, GLenum> TypeToSizeEnum()
{
	if constexpr (std::is_same_v<T, f32>)
	{
		return std::make_pair(1, GL_FLOAT);
	}
	if constexpr (std::is_same_v<T, s32>)
	{
		return std::make_pair(1, GL_INT);
	}
	if constexpr (std::is_same_v<T, s64>)
	{
		return std::make_pair(1, GL_UNSIGNED_INT);
	}
	if constexpr (std::is_same_v<T, glm::vec2>)
	{
		return std::make_pair(2, GL_FLOAT);
	}
	if constexpr (std::is_same_v<T, glm::vec3>)
	{
		return std::make_pair(3, GL_FLOAT);
	}
	if constexpr (std::is_same_v<T, glm::vec4>)
	{
		return std::make_pair(4, GL_FLOAT);
	}
	
	throw std::runtime_error("unsupported type");
}

template<typename T>
inline AttributeFormat CreateAttributeFormat(const u32 index, const u32 relativeOffset)
{
	const auto [componentCount, type] = TypeToSizeEnum<T>();
	return AttributeFormat{ index, componentCount, type, relativeOffset };
}
