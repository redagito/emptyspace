#pragma once

#include <emptyspace/types.hpp>
#include <emptyspace/graphics/attributeformat.hpp>

#include <vector>


class Geometry
{
public:
	template<typename T>
	Geometry(const std::vector<T>& vertices, const std::vector<u8>& indices, std::vector<AttributeFormat> vertexFormat)
	{
		_vertexBuffer = CreateBuffer(vertices);
		_indexBuffer = CreateBuffer(indices);

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
	
private:
	template<typename T>
	u32 CreateBuffer(const std::vector<T>& bufferData, u32 flags = GL_DYNAMIC_STORAGE_BIT)
	{
		u32 name = 0;

		glCreateBuffers(1, &name);
		glNamedBufferStorage(name, sizeof(typename std::vector<T>::value_type) * bufferData.size(), bufferData.data(), flags);

		return name;
	}

	u32 _vao;
	u32 _vertexBuffer;
	u32 _indexBuffer;
	std::vector<AttributeFormat> _vertexFormat;
};


