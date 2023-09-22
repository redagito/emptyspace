#pragma once
#include "types.hpp"

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
        _stride = sizeof(typename std::vector<T>::value_type);
        _size = static_cast<u32>(data.size());

        glCreateBuffers(1, &_id);
#ifdef _DEBUG
        char label[64];
        sprintf_s(label, "B_%s", typeid(T).name());
        glObjectLabel(GL_BUFFER, _id, static_cast<GLsizei>(strlen(label)), label);
#endif
        glNamedBufferStorage(_id, _stride * _size, data.data(), flags);
    }

    template <typename T>
    explicit Buffer(const u32 type, const std::vector<T>& data, const u32 flags = GL_DYNAMIC_STORAGE_BIT)
    {
        _stride = sizeof(typename std::vector<T>::value_type);
        _size = static_cast<u32>(data.size());

        glCreateBuffers(1, &_id);
#ifdef _DEBUG
        char label[64];
        sprintf_s(label, "B_%s", typeid(T).name());
        glObjectLabel(GL_BUFFER, _id, static_cast<GLsizei>(strlen(label)), label);
#endif
        glBindBuffer(type, _id);
        glBufferData(type, _stride * _size, data.data(), flags);
        glBindBuffer(type, 0);
    }

    ~Buffer();
    void BindAsStorageBuffer(const u32 bindingIndex) const;
    [[nodiscard]] u32 Id() const;
    [[nodiscard]] u32 Stride() const;
    [[nodiscard]] u32 Size() const;
private:
    u32 _id{};
    u32 _stride{};
    u32 _size{};
};
