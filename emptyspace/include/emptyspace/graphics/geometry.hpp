#pragma once
#define NOMINMAX
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



class Geometry final
{
public:
	static Geometry* CreateEmpty();
	static Geometry* CreateCube(f32 width, f32 height, f32 depth);
	static Geometry* CreatePlane(f32 width, f32 height);
	static Geometry* CreatePlainFromFile(const std::filesystem::path& filePath);
	static Geometry* CreateFromFile(const std::filesystem::path& filePath);
	void Bind() const;
	void DrawArrays() const;
	void DrawElements() const;

	~Geometry();
private:
	Geometry();
	
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
		//const auto label = typeid(T).name();
		//glObjectLabel(GL_BUFFER, _vao, strlen(label), label);
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

	template <typename T>
	[[nodiscard]] static constexpr std::pair<s32, u32> TypeToSize()
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
	static AttributeFormat CreateAttributeFormat(const u32 index, const u32 relateOffset)
	{
		auto const [comp_count, type] = TypeToSize<T>();
		return AttributeFormat{ index, comp_count, type, relateOffset };
	}

	u32 _vertexCount{};
	u32 _indexCount{};
	
	u32 _vao{};
	Buffer* _vbo{};
	Buffer* _ibo{};
};