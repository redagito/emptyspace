#pragma once
#define NOMINMAX
#include <emptyspace/types.hpp>
#include <emptyspace/graphics/buffer.hpp>
#include <emptyspace/graphics/vertexformats.hpp>

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <tuple>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <string>
#include <unordered_map>

struct AttributeFormat
{
    u32 Index;
    s32 Size;
    u32 Type;
    u32 RelativeOffset;
};

class Geometry final
{
public:
    static Geometry* CreateEmpty();
    static Geometry* CreateCube(const f32 width, const f32 height, const f32 depth);
    static Geometry* CreatePlane(const f32 width, const f32 height);
    static Geometry* CreatePlainFromFile(const std::filesystem::path& filePath);
    static Geometry* CreateFromFile(const std::filesystem::path& filePath);

    Geometry(
        const Buffer& vertexBuffer,
        const Buffer& indexBuffer,
        const enum VertexType vertexType)
    {
        glCreateVertexArrays(1, &_vao);
#ifdef _DEBUG
        char label[64];
        sprintf_s(label, "VAO_%s", _names[vertexType].c_str());
        glObjectLabel(0x8074, _vao, static_cast<GLsizei>(strlen(label)), label); // 0x8074 = GL_VERTEX_ARRAY
#endif
        glVertexArrayVertexBuffer(_vao, 0, vertexBuffer.Id(), 0, vertexBuffer.Stride());
        glVertexArrayElementBuffer(_vao, indexBuffer.Id());

        _vertexCount = vertexBuffer.Size();
        _indexCount = indexBuffer.Size();

        SetupInputLayout(vertexType);
    }

    void Bind() const;
    void Draw() const;
    void DrawInstanced(const u32 instanceCount) const;
    void DrawArrays() const;
    void DrawElements() const;

    ~Geometry();
private:
    Geometry();

    template <typename T>
    [[nodiscard]] static constexpr std::pair<s32, u32> TypeToSize()
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return std::make_pair(1, GL_FLOAT);
        }

        if constexpr (std::is_same_v<T, int>)
        {
            return std::make_pair(1, GL_INT);
        }

        if constexpr (std::is_same_v<T, unsigned int>)
        {
            return std::make_pair(1, GL_UNSIGNED_INT);
        }

        if constexpr (std::is_same_v<T, glm::vec2>)
        {
            return std::make_pair(2, GL_FLOAT);
        }

        if constexpr (std::is_same_v<T, glm::vec3>)
        {
            return std::make_pair(3, GL_FLOAT);
        }

        if constexpr (std::is_same_v<T, glm::vec4>)
        {
            return std::make_pair(4, GL_FLOAT);
        }

        throw std::runtime_error("GL: Unsupported type");
    }

    template <typename T>
    static AttributeFormat CreateAttributeFormat(const u32 index, const u32 relateOffset)
    {
        auto const [componentCount, type] = TypeToSize<T>();
        return AttributeFormat{ index, componentCount, type, relateOffset };
    }

    void SetupInputLayout(const enum VertexType vertexType) const
    {
        std::vector<AttributeFormat> attributes;
        switch (vertexType)
        {
        case VertexType::Position:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPosition, Position)));
        }
        break;
        case VertexType::PositionColorNormalUv:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)));
            attributes.push_back(CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Uv)));
        }
        break;
        case VertexType::PositionNormal:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionNormal, Position)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionNormal, Normal)));
        }
        break;
        case VertexType::PositionNormalUv:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionNormalUv, Position)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionNormalUv, Normal)));
            attributes.push_back(CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionNormalUv, Uv)));
        }
        break;
        case VertexType::PositionNormalUvTangent:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionNormalUvTangent, Position)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionNormalUvTangent, Normal)));
            attributes.push_back(CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionNormalUvTangent, Uv)));
            attributes.push_back(CreateAttributeFormat<glm::vec4>(4, offsetof(VertexPositionNormalUvTangent, Tangent)));
        }
        break;
        case VertexType::PositionNormalUvw:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionNormalUvw, Position)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionNormalUvw, Normal)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(3, offsetof(VertexPositionNormalUvw, Uvw)));
        }
        break;
        case VertexType::PositionNormalUvwTangent:
        {
            attributes.push_back(CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionNormalUvwTangent, Position)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionNormalUvwTangent, Normal)));
            attributes.push_back(CreateAttributeFormat<glm::vec3>(3, offsetof(VertexPositionNormalUvwTangent, Uvw)));
            attributes.push_back(CreateAttributeFormat<glm::vec4>(4, offsetof(VertexPositionNormalUvwTangent, Tangent)));
        }
        break;
        }

        for (auto& [index, size, type, relativeOffset] : attributes)
        {
            glEnableVertexArrayAttrib(_vao, index);
            glVertexArrayAttribFormat(_vao, index, size, type, GL_FALSE, relativeOffset);
            glVertexArrayAttribBinding(_vao, index, 0);
        }
    }

    u32 _vertexCount{};
    u32 _indexCount{};

    u32 _vao{};

    static inline std::unordered_map<VertexType, std::string> _names
    {
        { VertexType::Position, "Position" },
        { VertexType::PositionColorNormalUv, "PositionColorNormalUv" },
        { VertexType::PositionNormal, "PositionNormal" },
        { VertexType::PositionNormalUv, "PositionNormalUv" },
        { VertexType::PositionNormalUvTangent, "PositionNormalUvTangent" },
        { VertexType::PositionNormalUvw, "PositionNormalUvw" },
        { VertexType::PositionNormalUvwTangent, "PositionNormalUvwTangent" },
    };
};