#pragma once
#include <emptyspace/types.hpp>

#include <vector>
#include <glad/glad.h>

class InstanceBuffer final
{
public:
		
	InstanceBuffer()
	{
		glCreateBuffers(1, &_id);
	}
	
	~InstanceBuffer()
	{
		glDeleteBuffers(1, &_id);
	}

	void Bind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, _id, 0, _instanceDataSize);
	}
	
	template<typename T>
	void UpdateBuffer(const std::vector<T>& instances)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id);
		_instanceDataSize = sizeof(T) * instances.size();
		glBufferData(GL_SHADER_STORAGE_BUFFER, _instanceDataSize, instances.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
private:
	u32 _id{};
	std::size_t _instanceDataSize{};
};
