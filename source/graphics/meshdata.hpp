#pragma once

#include "types.hpp"
#include "graphics/vertexformats.hpp"

#include <filesystem>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Buffer;
class Geometry;

class MeshData
{
public:
	static MeshData* FromFile(const std::filesystem::path& filePath);

	MeshData();

	[[nodiscard]] VertexType VertexType() const;
	[[nodiscard]] u32 IndexCount() const;
	[[nodiscard]] u32 VertexCount() const;

	void AddPosition(const glm::vec3& position);
	void AddPositionNormal(const glm::vec3& position, const glm::vec3& normal);
	void AddPositionNormalUv(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv);
	void AddPositionNormalUvTangent(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv, const glm::vec3& tangent);
	void AddPositionNormalUvw(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& uvw);
	void AddPositionNormalUvwTangent(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& uvw, const glm::vec3& tangent);

	void AddFace(u32 index);
	void AddFace(const std::vector<u32>& indices);
	void AddFace(const u32 index0, const u32 index1, const u32 index2);

	template <typename TVertex>
	Geometry* BuildGeometry()
	{
		std::vector<TVertex> vertices;
		std::vector<u32> indices;
		for (unsigned index : _indices)
		{
			indices.emplace_back(index);
		}

		CalculateTangents();

		if constexpr (std::is_same_v<TVertex, VertexPosition>)
		{
			_vertexType = VertexType::Position;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i]);
			}
		}

		if constexpr (std::is_same_v<TVertex, VertexPositionColorNormalUv>)
		{
			_vertexType = VertexType::PositionColorNormalUv;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i], _colors[i], _normals[i], _uvs[i]);
			}
		}

		if constexpr (std::is_same_v<TVertex, VertexPositionNormal>)
		{
			_vertexType = VertexType::PositionNormal;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i], _normals[i]);
			}
		}

		if constexpr (std::is_same_v<TVertex, VertexPositionNormalUv>)
		{
			_vertexType = VertexType::PositionNormalUv;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i], _normals[i], _uvs[i]);
			}
		}

		if constexpr (std::is_same_v<TVertex, VertexPositionNormalUvTangent>)
		{
			_vertexType = VertexType::PositionNormalUvTangent;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i], _normals[i], _uvs[i], _realTangents[i]);
			}
		}

		if constexpr (std::is_same_v<TVertex, VertexPositionNormalUvw>)
		{
			_vertexType = VertexType::PositionNormalUvw;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i], _normals[i], _uvws[i]);
			}
		}

		if constexpr (std::is_same_v<TVertex, VertexPositionNormalUvwTangent>)
		{
			_vertexType = VertexType::PositionNormalUvwTangent;
			for (auto i = 0; i < _positions.size(); i++)
			{
				vertices.emplace_back(_positions[i], _normals[i], _uvws[i], _realTangents[i]);
			}
		}

		const auto vertexBuffer = new Buffer(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
		const auto indexBuffer = new Buffer(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW);

		return new Geometry(*vertexBuffer, *indexBuffer, _vertexType);
	}
private:
	void CalculateTangents();

	std::vector<glm::vec3> _positions;
	std::vector<glm::vec3> _colors;
	std::vector<glm::vec3> _normals;
	std::vector<glm::vec2> _uvs;
	std::vector<glm::vec3> _uvws;
	std::vector<glm::vec3> _tangents;
	std::vector<glm::vec3> _bitangents;
	std::vector<glm::vec4> _realTangents;
	std::vector<unsigned> _indices;
	enum VertexType _vertexType { VertexType::Position };
};
