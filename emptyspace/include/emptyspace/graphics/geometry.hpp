#pragma once

#include <emptyspace/types.hpp>
#include <emptyspace/graphics/attributeformat.hpp>

#include <vector>
#include <string>

class Geometry
{
public:
	template <typename T>
	Geometry(const std::vector<T>& vertices, const std::vector<u8>& indices, std::vector<AttributeFormat> vertexFormat)
	{
		_vertexBuffer = CreateBuffer(vertices);
		_vertexCount = u32(vertices.size());
		_indexBuffer = CreateBuffer(indices);
		_indexCount = u32(indices.size());

		glCreateVertexArrays(1, &_vao);
		glVertexArrayVertexBuffer(_vao, 0, _vertexBuffer, 0, sizeof(T));
		glVertexArrayElementBuffer(_vao, _indexBuffer);

		for (const auto& format : vertexFormat)
		{
			glEnableVertexArrayAttrib(_vao, format.Index);
			glVertexArrayAttribFormat(_vao, format.Index, format.Size, format.Type, false, format.RelativeOffset);
			glVertexArrayAttribBinding(_vao, format.Index, 0);
		}
	}

	~Geometry()
	{
		glDeleteBuffers(1, &_indexBuffer);
		glDeleteBuffers(1, &_vertexBuffer);
		glDeleteVertexArrays(1, &_vao);
	}

	inline void Bind() const
	{
		glBindVertexArray(_vao);
	}

	inline void Draw() const
	{
		glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
	}

	inline void DrawElements() const
	{
		glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_BYTE, nullptr);
	}

	inline void DrawInstanced(const s32 instanceCount) const
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, _vertexCount, instanceCount);
	}

	inline void DrawElementsInstanced(const s32 instanceCount) const
	{
		glDrawElementsInstanced(GL_TRIANGLES, _indexCount, GL_UNSIGNED_BYTE, nullptr, instanceCount);
	}

private:
	template <typename T>
	static u32 CreateBuffer(const std::vector<T>& bufferData, u32 flags = GL_DYNAMIC_STORAGE_BIT)
	{
		u32 name = 0;

		glCreateBuffers(1, &name);
		glNamedBufferStorage(name, sizeof(typename std::vector<T>::value_type) * bufferData.size(), bufferData.data(), flags);

		return name;
	}

	u32 _vao;
	u32 _vertexBuffer;
	u32 _indexBuffer;
	u32 _indexCount;
	u32 _vertexCount;
	std::vector<AttributeFormat> _vertexFormat;
};
