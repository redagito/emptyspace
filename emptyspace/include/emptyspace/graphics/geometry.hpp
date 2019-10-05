#pragma once
#include <emptyspace/types.hpp>
#include <emptyspace/graphics/buffer.hpp>

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <tuple>
#include <vector>
#include <stdexcept>
#include <filesystem>

struct VertexPosition
{
	glm::vec3 Position;

	explicit VertexPosition(const glm::vec3& position)
		: Position(position)
	{}
};

struct VertexPositionColorNormalUv
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Normal;
	glm::vec2 Texcoord;

	explicit VertexPositionColorNormalUv(glm::vec3 const& position, glm::vec3 const& color, glm::vec3 const& normal, glm::vec2 const& texcoord)
		: Position(position), Color(color), Normal(normal), Texcoord(texcoord)
	{
	}
};

struct AttributeFormat
{
	u32 Index;
	s32 Size;
	u32 Type;
	u32 RelativeOffset;
};

template <typename T>
constexpr std::pair<s32, u32> TypeToSize()
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

class Geometry final
{
public:
	static Geometry* CreateEmpty()
	{
		return new Geometry();
	}
	
	static Geometry* CreateCube(f32 width, f32 height, f32 depth)
	{
		std::vector<VertexPositionColorNormalUv> const vertices =
		{
			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),

			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),

			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),

			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

			VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		};

		std::vector<u8> const indices =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,

			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20,
		};

		const auto vertexCount = vertices.size();
		const auto indexCount = indices.size();

		std::vector<AttributeFormat> const vertexFormat =
		{
			CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
			CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
			CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
			CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Texcoord))
		};

		return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
	}
	
	static Geometry* CreatePlane(f32 width, f32 height)
	{
		std::vector<VertexPositionColorNormalUv> const vertices =
		{
			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			VertexPositionColorNormalUv(glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
			VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		};

		std::vector<u8> const indices =
		{
			0, 1, 2, 2, 3, 0,
		};

		const auto vertexCount = vertices.size();
		const auto indexCount = indices.size();

		std::vector<AttributeFormat> const vertexFormat =
		{
			CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
			CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
			CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
			CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Texcoord))
		};

		return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
	}
	
	static Geometry* CreateFromFilePlain(const std::filesystem::path& filePath)
	{
		std::vector<VertexPosition> vertices;
		std::vector<u8> indices;

		Assimp::Importer importer;
		const auto scene = importer.ReadFile(filePath.string(), /*aiProcess_JoinIdenticalVertices |*/ aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FindInvalidData);
		const auto mesh = scene->mMeshes[0];

		for (u32 f = 0; f < mesh->mNumFaces; ++f)
		{
			const auto face = mesh->mFaces[f];
			if (face.mNumIndices != 3)
			{
				continue;
			}

			for (u32 i = 0; i < 3; ++i)
			{
				const auto index = face.mIndices[i];

				glm::vec3 position;
				position.x = mesh->mVertices[index].x;
				position.y = mesh->mVertices[index].y;
				position.z = mesh->mVertices[index].z;

				VertexPosition vertex(position);
				vertices.push_back(vertex);

				indices.push_back(index);
			}
		}

		const auto vertexCount = vertices.size();
		const auto indexCount = indices.size();

		std::vector<AttributeFormat> const vertexFormat =
		{
			CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPosition, Position))
		};

		return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
	}

	static Geometry* CreateFromFile(const std::filesystem::path& filePath)
	{
		std::vector<VertexPositionColorNormalUv> vertices;
		std::vector<u8> indices;

		Assimp::Importer importer;
		const auto scene = importer.ReadFile(filePath.string(), /*aiProcess_JoinIdenticalVertices |*/ aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FindInvalidData);
		const auto mesh = scene->mMeshes[0];

		for (u32 f = 0; f < mesh->mNumFaces; ++f)
		{
			const auto face = mesh->mFaces[f];
			if (face.mNumIndices != 3)
			{
				continue;
			}

			for (u32 i = 0; i < 3; ++i)
			{
				const auto index = face.mIndices[i];

				glm::vec3 position;
				position.x = mesh->mVertices[index].x;
				position.y = mesh->mVertices[index].y;
				position.z = mesh->mVertices[index].z;

				glm::vec3 normal;
				if (mesh->HasNormals())
				{
					normal.x = mesh->mNormals[index].x;
					normal.y = mesh->mNormals[index].y;
					normal.z = mesh->mNormals[index].z;
				}

				glm::vec2 uv(0.0f, 0.0f);

				VertexPositionColorNormalUv vertex(position, normal, normal, uv);
				vertices.push_back(vertex);
			}
		}

		const auto vertexCount = vertices.size();
		const auto indexCount = indices.size();

		std::vector<AttributeFormat> const vertexFormat =
		{
			CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
			CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
			CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
			CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Texcoord))
		};

		return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
	}

	void Bind() const
	{
		glBindVertexArray(_vao);
	}

	void DrawArrays() const
	{
		glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
	}

	void DrawElements() const
	{
		glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_BYTE, nullptr);
	}

	~Geometry()
	{
		delete _ibo;
		delete _vbo;
		glDeleteVertexArrays(1, &_vao);
	}
private:
	Geometry()
	{
		glCreateVertexArrays(1, &_vao);
	}
	
	template <typename T>
	Geometry(const std::vector<T>& vertices, const std::vector<u8>& indices, u32 vertexCount, u32 indexCount, const std::vector<AttributeFormat>& attributeFormats)
	{
		glCreateVertexArrays(1, &_vao);
		
		_vbo = new Buffer(vertices, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
		_ibo = new Buffer(indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

		_vertexCount = vertexCount;
		_indexCount = indexCount;

		glCreateVertexArrays(1, &_vao);
#ifdef _DEBUG
		const auto label = typeid(T).name();
		glObjectLabel(GL_BUFFER, _vao, strlen(label), label);
#endif
		glVertexArrayVertexBuffer(_vao, 0, _vbo->GetName(), 0, sizeof(T));
		glVertexArrayElementBuffer(_vao, _ibo->GetName());

		for (auto const& format : attributeFormats)
		{
			glEnableVertexArrayAttrib(_vao, format.Index);
			glVertexArrayAttribFormat(_vao, format.Index, format.Size, format.Type, GL_FALSE, format.RelativeOffset);
			glVertexArrayAttribBinding(_vao, format.Index, 0);
		}
	}

	u32 _vertexCount{};
	u32 _indexCount{};
	
	u32 _vao{};
	Buffer* _vbo{};
	Buffer* _ibo{};
};