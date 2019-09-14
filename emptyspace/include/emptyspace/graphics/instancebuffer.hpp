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
		glBindBuffer(GL_ARRAY_BUFFER, _id);
	}
	
	template<typename T>
	void UpdateBuffer(const std::vector<T>& instances)
	{
		glBindBuffer(GL_ARRAY_BUFFER, _id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(T) * instances.size(), instances.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
private:
	u32 _id{};
};
