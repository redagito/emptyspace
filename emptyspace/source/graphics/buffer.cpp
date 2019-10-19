#include <emptyspace/graphics/buffer.hpp>

Buffer::Buffer()
{
	glCreateBuffers(1, &_name);
}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &_name);
}

void Buffer::BindAsStorageBuffer() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _name);
}

u32 Buffer::GetName() const
{
	return _name;
}