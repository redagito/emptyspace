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
	}
	
	template<typename T>
	void UpdateBuffer(const std::vector<T>& instances)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(typename std::vector<T>::value_type) * instances.size(), instances.data(), GL_DYNAMIC_DRAW);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	u32 GetId() const
	{
		return _id;
	}
private:
	u32 _id{};
};
