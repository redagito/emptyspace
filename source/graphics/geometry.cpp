#define NOMINMAX
#include "graphics/geometry.hpp"
#include "graphics/meshdata.hpp"

Geometry* Geometry::CreateEmpty()
{
    return new Geometry();
}

Geometry* Geometry::CreateUnitCube()
{
    auto meshData = new MeshData();
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f));

    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));

    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    meshData->AddPositionNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

    std::vector<u32> const indices =
    {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,

        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
    meshData->AddFace(indices);
    return meshData->BuildGeometry<VertexPositionNormalUvwTangent>();
}

Geometry* Geometry::CreateUnitPlane()
{
    auto meshData = new MeshData();
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    meshData->AddPositionNormalUv(glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    meshData->AddPositionNormalUv(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    meshData->AddFace(0, 1, 2);
    meshData->AddFace(2, 3, 0);

    return meshData->BuildGeometry<VertexPositionNormalUv>();
}

Geometry* Geometry::CreatePlainFromFile(const std::filesystem::path& filePath)
{
    auto meshData = MeshData::FromFile(filePath);
    return meshData->BuildGeometry<VertexPosition>();
}

Geometry* Geometry::CreateFromFile(const std::filesystem::path& filePath)
{
    auto meshData = MeshData::FromFile(filePath);
    switch (meshData->GetVertexType())
    {
        case VertexType::Position: return meshData->BuildGeometry<VertexPosition>();
        case VertexType::PositionColorNormalUv: return meshData->BuildGeometry<VertexPositionColorNormalUv>();
        case VertexType::PositionNormal: return meshData->BuildGeometry<VertexPositionNormal>();
        case VertexType::PositionNormalUv: return meshData->BuildGeometry<VertexPositionNormalUv>();
        case VertexType::PositionNormalUvTangent: return meshData->BuildGeometry<VertexPositionNormalUvTangent>();
        default: throw std::runtime_error("Invalid vertex type");
    }
}

void Geometry::Bind() const
{
    glBindVertexArray(_vao);
}

void Geometry::Draw() const
{
    if (_indexCount == 0 || _indexCount == _vertexCount)
    {
        DrawArrays();
    }
    else
    {
        DrawElements();
    }
}

void Geometry::DrawInstanced(const u32 instanceCount) const
{
    if (_indexCount == 0)
    {
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, _vertexCount, instanceCount, 0);
    }
    else
    {
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr, instanceCount, 0);
    }
}

void Geometry::DrawArrays() const
{
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, _vertexCount, 1, 0);
}

void Geometry::DrawElements() const
{
    glDrawElementsInstancedBaseInstance(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr, 1, 0);
}

Geometry::Geometry()
{
    glCreateVertexArrays(1, &_vao);
#ifdef _DEBUG
    const auto label = "VAO_Empty";
    glObjectLabel(0x8074, _vao, static_cast<GLsizei>(strlen(label)), label); // 0x8074 = GL_VERTEX_ARRAY
#endif
}

Geometry::~Geometry()
{
    glDeleteVertexArrays(1, &_vao);
}