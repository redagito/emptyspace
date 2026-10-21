#include "types.hpp"
#include "graphics/buffer.hpp"
#include "graphics/meshdata.hpp"
#include "graphics/geometry.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace
{
	// A unit vector perpendicular to the normal, for tangents that are
	// parallel to it.
	glm::vec3 AnyPerpendicular(const glm::vec3& normal)
	{
		const auto axis = glm::abs(normal.x) < 0.9f
			? glm::vec3(1.0f, 0.0f, 0.0f)
			: glm::vec3(0.0f, 1.0f, 0.0f);
		return glm::normalize(glm::cross(normal, axis));
	}
}

MeshData* MeshData::FromFile(const std::filesystem::path& filePath)
{
    auto meshData = new MeshData();
    Assimp::Importer importer;

    auto constexpr importerFlags = aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate | aiProcess_FixInfacingNormals |
        aiProcess_FindInvalidData | aiProcess_OptimizeMeshes;
    const auto scene = importer.ReadFile(filePath.string(), importerFlags);
    const auto mesh = scene->mMeshes[0];

    const auto hasNormals = mesh->HasNormals();
    const auto hasTextureCoordinates = mesh->HasTextureCoords(0);
    const auto hasTangents = mesh->HasTangentsAndBitangents();

    if (!hasNormals)
    {
        meshData->_vertexType = VertexType::Position;
    }
    else if (!hasTextureCoordinates)
    {
        meshData->_vertexType = VertexType::PositionNormal;
    }
    else if (!hasTangents)
    {
        meshData->_vertexType = VertexType::PositionNormalUv;
    }
    else
    {
        meshData->_vertexType = VertexType::PositionNormalUvTangent;
    }

    for (u32 f = 0; f < mesh->mNumFaces; ++f)
    {
        const auto& face = mesh->mFaces[f];
        if (face.mNumIndices != 3)
        {
            continue;
        }

        // The vertices are appended per face, so the indices of the triangle
        // soup are consecutive.
        const auto base = static_cast<u32>(meshData->VertexCount());
        meshData->AddFace(base, base + 1, base + 2);

        for (u32 i = 0; i < 3; ++i)
        {
            const auto index = face.mIndices[i];

            glm::vec3 position{};
            position.x = mesh->mVertices[index].x;
            position.y = mesh->mVertices[index].y;
            position.z = mesh->mVertices[index].z;

            glm::vec3 normal{};
            if (hasNormals)
            {
                normal.x = mesh->mNormals[index].x;
                normal.y = mesh->mNormals[index].y;
                normal.z = mesh->mNormals[index].z;

                glm::vec3 uv{};
                if (hasTextureCoordinates)
                {
                    uv.r = mesh->mTextureCoords[0]->x;
                    uv.s = mesh->mTextureCoords[0]->y;

                    if (hasTangents)
                    {
                        glm::vec3 tangent{};
                        tangent.x = mesh->mTangents[index].x;
                        tangent.y = mesh->mTangents[index].y;
                        tangent.z = mesh->mTangents[index].z;

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

    for (size_t i = 0; i + 2 < _indices.size(); i += 3)
    {
        const auto i0 = _indices[i];
        const auto i1 = _indices[i + 1];
        const auto i2 = _indices[i + 2];

        const auto edge1 = _positions[i1] - _positions[i0];
        const auto edge2 = _positions[i2] - _positions[i0];

        const auto uv1 = _uvs[i1] - _uvs[i0];
        const auto uv2 = _uvs[i2] - _uvs[i0];

        const auto determinant = uv1.x * uv2.y - uv2.x * uv1.y;
        if (glm::abs(determinant) <= FLT_EPSILON)
        {
            continue;
        }

        const auto inverseDeterminant = 1.0f / determinant;

        const auto tangent = (uv2.y * edge1 - uv1.y * edge2) * inverseDeterminant;
        const auto bitangent = (uv1.x * edge2 - uv2.x * edge1) * inverseDeterminant;

        _tangents[i0] += tangent;
        _tangents[i1] += tangent;
        _tangents[i2] += tangent;

        _bitangents[i0] += bitangent;
        _bitangents[i1] += bitangent;
        _bitangents[i2] += bitangent;
    }

    _realTangents.clear();
    _realTangents.reserve(_positions.size());

    for (size_t i = 0; i < _positions.size(); ++i)
    {
        const auto& normal = _normals[i];
        const auto& tangent = _tangents[i];

        auto orthogonal = tangent - (normal * glm::dot(normal, tangent));
        if (glm::dot(orthogonal, orthogonal) <= FLT_EPSILON)
        {
            // The tangent is parallel to the normal, any perpendicular will do.
            orthogonal = AnyPerpendicular(normal);
        }

        const auto unitTangent = glm::normalize(orthogonal);
        const auto handedness = glm::dot(glm::cross(normal, unitTangent), _bitangents[i]) < 0.0f
            ? -1.0f
            : 1.0f;
        _realTangents.emplace_back(unitTangent, handedness);
    }
}