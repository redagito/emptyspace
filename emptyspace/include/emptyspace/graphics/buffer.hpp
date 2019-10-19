#pragma once
#include <emptyspace/types.hpp>

#include <glad/glad.h>

#include <vector>
#include <typeinfo>

class Buffer final
{
public:
	Buffer();

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

	~Buffer();
	void BindAsStorageBuffer() const;
	u32 GetName() const;
private:
	u32 _name{};
};
