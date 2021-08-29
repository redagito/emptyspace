#include <emptyspace/graphics/buffer.hpp>

Buffer::Buffer()
{
    glCreateBuffers(1, &_id);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &_id);
}

void Buffer::BindAsStorageBuffer(const u32 bindingIndex) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, _id);
}

[[nodiscard]] u32 Buffer::Id() const
{
    return _id;
}

[[nodiscard]] u32 Buffer::Stride() const
{
    return _stride;
}

[[nodiscard]] u32 Buffer::Size() const
{
    return _size;
}