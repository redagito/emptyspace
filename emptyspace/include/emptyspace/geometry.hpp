#pragma once
#include <emptyspace/types.hpp>

#include <glad/glad.h>

#include <tuple>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <stdexcept>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Normal;
	glm::vec2 Texcoord;

	Vertex(glm::vec3 const& position, glm::vec3 const& color, glm::vec3 const& normal, glm::vec2 const& texcoord)
		: Position(position), Color(color), Normal(normal), Texcoord(texcoord)
	{
	}
};

struct AttributeFormat
{
	GLuint Index;
	GLint Size;
	GLenum Type;
	GLuint RelativeOffset;
};

template <typename T>
constexpr std::pair<GLint, GLenum> TypeToSize()
{
	if constexpr (std::is_same_v<T, float>)
	{
		return std::make_pair(1, GL_FLOAT);
	}

	if constexpr (std::is_same_v<T, int>)
	{
		return std::make_pair(1, GL_INT);
	}

	if constexpr (std::is_same_v<T, unsigned int>)
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

	throw std::runtime_error("GL: Unsupported type");
}

template <typename T>
AttributeFormat CreateAttributeFormat(const u32 index, const u32 relateOffset)
{
	auto const [comp_count, type] = TypeToSize<T>();
	return AttributeFormat{ index, comp_count, type, relateOffset };
}

template <typename T>
u32 CreateBuffer(std::vector<T> const& buff, const u32 flags = GL_DYNAMIC_STORAGE_BIT)
{
	GLuint name = 0;
	glCreateBuffers(1, &name);
	glNamedBufferStorage(name, sizeof(typename std::vector<T>::value_type) * buff.size(), buff.data(), flags);
	return name;
}

template <typename T>
std::tuple<GLuint, GLuint, GLuint> CreateGeometry(const std::vector<T>& vertices, const std::vector<u8>& indices, const std::vector<AttributeFormat>& attributeFormats)
{
	u32 vao = 0;
	auto vbo = CreateBuffer(vertices, GL_MAP_READ_BIT);
	auto ibo = CreateBuffer(indices, GL_MAP_READ_BIT);

	glCreateVertexArrays(1, &vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(T));
	glVertexArrayElementBuffer(vao, ibo);

	for (auto const& format : attributeFormats)
	{
		glEnableVertexArrayAttrib(vao, format.Index);
		glVertexArrayAttribFormat(vao, format.Index, format.Size, format.Type, GL_FALSE, format.RelativeOffset);
		glVertexArrayAttribBinding(vao, format.Index, 0);
	}

	return std::make_tuple(vao, vbo, ibo);
}