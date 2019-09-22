#pragma once
#include <emptyspace/types.hpp>

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <tuple>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <glm/vec4.hpp>

namespace std {
	namespace filesystem {
		class path;
	}
}

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
u32 CreateBuffer(std::vector<T> const& buff, const u32 type, const u32 flags = GL_DYNAMIC_STORAGE_BIT)
{
	GLuint name = 0;
	glCreateBuffers(1, &name);
	glBindBuffer(type, name);
	glBufferData(type, sizeof(typename std::vector<T>::value_type) * buff.size(), buff.data(), flags);
	glBindBuffer(type, 0);
	return name;
}

template <typename T>
std::tuple<u32, u32, u32> CreateGeometry(const std::vector<T>& vertices, const std::vector<u8>& indices, const std::vector<AttributeFormat>& attributeFormats)
{
	u32 vao = 0;
	auto vbo = CreateBuffer(vertices, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	auto ibo = CreateBuffer(indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

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

inline std::tuple<u32, u32, u32, u32, u32> CreateGeometryFromFile(const std::filesystem::path& filePath, const std::vector<AttributeFormat>& attributeFormats)
{
	u32 vao = 0;

	std::vector<Vertex> vertices;
	std::vector<u8> indices;

	Assimp::Importer importer;
	const auto scene = importer.ReadFile(filePath.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FindInvalidData);
	const auto mesh = scene->mMeshes[0];
	//const auto mesh = scene->mMeshes[0];
	//for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	//{
	//	glm::vec3 position;
	//	// positions
	//	position.x = mesh->mVertices[i].x;
	//	position.y = mesh->mVertices[i].y;
	//	position.z = mesh->mVertices[i].z;

	//	glm::vec3 normal;
	//	// normals
	//	normal.x = mesh->mNormals[i].x;
	//	normal.y = mesh->mNormals[i].y;
	//	normal.z = mesh->mNormals[i].z;

	//	glm::vec2 uv(0.0f, 0.0f);

	//	Vertex vertex(position, normal, normal, uv);
	//	vertices.push_back(vertex);
	//}
	//
	//for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	//{
	//	const auto face = mesh->mFaces[i];
	//	for (unsigned int j = 0; j < face.mNumIndices; j++)
	//	{
	//		indices.push_back(face.mIndices[j]);
	//	}
	//}
	//

	for (u32 f = 0; f<mesh->mNumFaces; ++f)
	{
		const auto face = mesh->mFaces[f];
		if (face.mNumIndices != 3)
		{
			continue;
		}

		for (u32 i = 0; i < 3;++i)
		{
			const auto index = face.mIndices[i];

			glm::vec3 position;
		    position.x = mesh->mVertices[index].x;
			position.y = mesh->mVertices[index].y;
			position.z = mesh->mVertices[index].z;

			glm::vec3 normal;
			normal.x = mesh->mNormals[index].x;
			normal.y = mesh->mNormals[index].y;
			normal.z = mesh->mNormals[index].z;

			glm::vec2 uv(0.0f, 0.0f);

			Vertex vertex(position, normal, normal, uv);
			vertices.push_back(vertex);
		}
	}

	auto vbo = CreateBuffer(vertices, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	auto ibo = CreateBuffer(indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

	glCreateVertexArrays(1, &vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vao, ibo);

	for (auto const& format : attributeFormats)
	{
		glEnableVertexArrayAttrib(vao, format.Index);
		glVertexArrayAttribFormat(vao, format.Index, format.Size, format.Type, GL_FALSE, format.RelativeOffset);
		glVertexArrayAttribBinding(vao, format.Index, 0);
	}

	return std::make_tuple(vao, vbo, ibo, vertices.size(), indices.size());
	
}
