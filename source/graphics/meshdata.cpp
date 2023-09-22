#include "types.hpp"
#include "graphics/buffer.hpp"
#include "graphics/meshdata.hpp"
#include "graphics/geometry.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

MeshData* MeshData::FromFile(const std::filesystem::path& filePath)
{
    auto meshData = new MeshData();
    Assimp::Importer importer;

    auto constexpr importerFlags = aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate | aiProcess_FixInfacingNormals |
        aiProcess_FindInvalidData | aiProcess_OptimizeMeshes;
    const auto scene = importer.ReadFile(filePath.string(), importerFlags);
    const auto mesh = scene->mMeshes[0];

    for (u32 f = 0; f < mesh->mNumFaces; ++f)
    {
        const auto& face = mesh->mFaces[f];
        if (face.mNumIndices != 3)
        {
            continue;
        }

        meshData->AddFace(face.mIndices[0]);
        meshData->AddFace(face.mIndices[1]);
        meshData->AddFace(face.mIndices[2]);

        for (u32 i = 0; i < 3; ++i)
        {
            const auto index = face.mIndices[i];

            glm::vec3 position{};
            position.x = mesh->mVertices[index].x;
            position.y = mesh->mVertices[index].y;
            position.z = mesh->mVertices[index].z;

            glm::vec3 normal{};
            if (mesh->HasNormals())
            {
                normal.x = mesh->mNormals[index].x;
                normal.y = mesh->mNormals[index].y;
                normal.z = mesh->mNormals[index].z;

                glm::vec3 uv{};
                if (mesh->HasTextureCoords(0))
                {
                    uv.r = mesh->mTextureCoords[0]->x;
                    uv.s = mesh->mTextureCoords[0]->y;

                    if (mesh->HasTangentsAndBitangents())
                    {
                        glm::vec3 tangent{};
                        tangent.x = mesh->mTangents[i].x;
                        tangent.y = mesh->mTangents[i].y;
                        tangent.z = mesh->mTangents[i].z;

                        meshData->AddPositionNormalUvTangent(position, normal, uv, glm::vec4(tangent, 1.0f));
                    }
                    else
                    {
                        meshData->AddPositionNormalUv(position, normal, uv);
                    }
                }
                else
                {
                    meshData->AddPositionNormal(position, normal);
                }
            }
            else
            {
                meshData->AddPosition(position);
            }

        }
    }

    return meshData;
}

MeshData::MeshData()
{
}

void MeshData::AddPosition(const glm::vec3& position)
{
    _positions.emplace_back(position);
}

void MeshData::AddPositionNormal(const glm::vec3& position, const glm::vec3& normal)
{
    _positions.emplace_back(position);
    _normals.emplace_back(normal);
}

void MeshData::AddPositionNormalUv(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv)
{
    _positions.emplace_back(position);
    _normals.emplace_back(normal);
    _uvs.emplace_back(uv);
    _uvws.emplace_back(glm::vec3(uv, -1.0f));
    _tangents.emplace_back(glm::vec3{});
    _bitangents.emplace_back(glm::vec3{});
}

void MeshData::AddPositionNormalUvTangent(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv, const glm::vec3& tangent)
{
    _positions.emplace_back(position);
    _normals.emplace_back(normal);
    _uvs.emplace_back(uv);
    _uvws.emplace_back(glm::vec3(uv, -1.0f));
    _tangents.emplace_back(tangent);
    _bitangents.emplace_back(glm::vec3{});
}

void MeshData::AddPositionNormalUvw(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& uvw)
{
    _positions.emplace_back(position);
    _normals.emplace_back(normal);
    _uvs.emplace_back(uvw.x, uvw.y);
    _uvws.emplace_back(uvw);
    _tangents.emplace_back(glm::vec3{});
    _bitangents.emplace_back(glm::vec3{});
}

void MeshData::AddPositionNormalUvwTangent(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& uvw, const glm::vec3& tangent)
{
    _positions.emplace_back(position);
    _normals.emplace_back(normal);
    _uvs.emplace_back(uvw.x, uvw.y);
    _uvws.emplace_back(uvw);
    _tangents.emplace_back(tangent);
    _bitangents.emplace_back(glm::vec3{});
}

void MeshData::AddFace(u32 index)
{
    _indices.emplace_back(index);
}

void MeshData::AddFace(const std::vector<u32>& indices)
{
    for (unsigned index : indices)
    {
        _indices.emplace_back(index);
    }
}

void MeshData::AddFace(const u32 index0, const u32 index1, const u32 index2)
{
    _indices.emplace_back(index0);
    _indices.emplace_back(index1);
    _indices.emplace_back(index2);
}

[[nodiscard]] VertexType MeshData::GetVertexType() const
{
    return _vertexType;
}

u32 MeshData::IndexCount() const
{
    return static_cast<u32>(_indices.size());
}

u32 MeshData::VertexCount() const
{
    return static_cast<u32>(_positions.size());
}

void MeshData::CalculateTangents()
{
    if (_tangents.empty())
    {
        return;
    }

    for (auto& normal : _normals)
    {
        normal = glm::normalize(normal);
    }

    for (auto i = 0; i < _positions.size(); i += 3)
    {
        if (i >= _positions.size() || i + 1 >= _positions.size() || i + 2 >= _positions.size())
        {
            break;
        }
        auto triangle = glm::mat3x3();
        triangle[0] = _positions[i + 0];
        triangle[1] = _positions[i + 1];
        triangle[2] = _positions[i + 2];

        const auto uv0 = glm::vec2(_uvs[i + 1].x - _uvs[i + 0].x, _uvs[i + 2].x - _uvs[i + 0].x);
        const auto uv1 = glm::vec2(_uvs[i + 1].y - _uvs[i + 0].y, _uvs[i + 2].y - _uvs[i + 0].y);

        const auto q1 = triangle[1] - triangle[0];
        const auto q2 = triangle[2] - triangle[0];

        auto determinant = uv0.x * uv1.y - uv1.x * uv0.y;
        if (glm::abs(determinant) <= FLT_EPSILON)
        {
            determinant = 0.000001f;
        }

        const auto inverseDeterminant = 1.0f / determinant;

        const auto tangent = glm::vec3
        {
            inverseDeterminant * (uv1.y * q1.x - uv0.y * q2.x),
            inverseDeterminant * (uv1.y * q1.y - uv0.y * q2.y),
            inverseDeterminant * (uv1.y * q1.z - uv0.y * q2.z)
        };
        const auto bitangent = glm::vec3
        {
            inverseDeterminant * (-uv1.x * q1.x * uv0.x * q2.x),
            inverseDeterminant * (-uv1.x * q1.y * uv0.x * q2.y),
            inverseDeterminant * (-uv1.x * q1.z * uv0.x * q2.z)
        };

        _tangents[i + 0] += tangent;
        _tangents[i + 1] += tangent;
        _tangents[i + 2] += tangent;

        _bitangents[i + 0] += bitangent;
        _bitangents[i + 1] += bitangent;
        _bitangents[i + 2] += bitangent;
    }

    for (int i = 0; i < _positions.size(); ++i)
    {
        auto& normal = _normals[i];
        auto& tangent = _tangents[i];
        auto& bitangent = _bitangents[i];

        auto realTangent = glm::normalize(tangent - (normal * glm::dot(normal, tangent)));
        auto realBitangent = glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f
            ? -1.0f
            : 1.0f;
        _realTangents.emplace_back(realTangent, realBitangent);
    }
}