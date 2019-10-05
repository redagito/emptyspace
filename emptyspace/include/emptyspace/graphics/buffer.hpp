#pragma once
#include <emptyspace/types.hpp>

#include <glad/glad.h>

#include <vector>

class Buffer final
{
public:
	Buffer()
	{
		glCreateBuffers(1, &_name);
	}

	template <typename T>
	explicit Buffer(const std::vector<T>& data, const u32 flags = GL_DYNAMIC_STORAGE_BIT)
	{
		glCreateBuffers(1, &_name);
#ifdef _DEBUG
		const auto label = typeid(T).name();
		glObjectLabel(GL_BUFFER, _name, strlen(label), label);
#endif
		glNamedBufferStorage(_name, sizeof(typename std::vector<T>::value_type) * data.size(), data.data(), flags);
	}

	template <typename T>
	explicit Buffer(const std::vector<T>& data, const u32 type, const u32 flags = GL_DYNAMIC_STORAGE_BIT)
	{
		glCreateBuffers(1, &_name);
#ifdef _DEBUG
		const auto label = typeid(T).name();
		glObjectLabel(GL_BUFFER, _name, strlen(label), label);
#endif
		glBindBuffer(type, _name);
		glBufferData(type, sizeof(typename std::vector<T>::value_type) * data.size(), data.data(), flags);
		glBindBuffer(type, 0);
	}

	~Buffer()
	{
		glDeleteBuffers(1, &_name);
	}

	void BindAsStorageBuffer() const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _name);
	}

	u32 GetName() const
	{
		return _name;
	}
private:
	u32 _name{};
};
