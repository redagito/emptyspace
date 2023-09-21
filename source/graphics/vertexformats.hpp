#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

enum class VertexType
{
    Position,
    PositionColorNormalUv,
    PositionNormal,
    PositionNormalUv,
    PositionNormalUvTangent,
    PositionNormalUvw,
    PositionNormalUvwTangent
};

struct VertexPosition
{
    glm::vec3 Position;

    explicit VertexPosition(
        const glm::vec3& position)
        : Position(position)
    {
    }
};

struct VertexPositionColorNormalUv
{
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec3 Normal;
    glm::vec2 Uv;

    explicit VertexPositionColorNormalUv(
        glm::vec3 const& position,
        glm::vec3 const& color,
        glm::vec3 const& normal,
        glm::vec2 const& uv)
        : Position(position), Color(color), Normal(normal), Uv(uv)
    {
    }
};

struct VertexPositionNormal
{
    glm::vec3 Position;
    glm::vec3 Normal;

    explicit VertexPositionNormal(
        const glm::vec3& position,
        const glm::vec3& normal)
            : Position(position), Normal(normal)
    {
    }
};

struct VertexPositionNormalUv
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 Uv;

    explicit VertexPositionNormalUv(
        const glm::vec3 position,
        const glm::vec3 normal,
        const glm::vec2 uv)
            : Position(position), Normal(normal), Uv(uv)
    {
    }
};

struct VertexPositionNormalUvTangent
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 Uv;
    glm::vec4 Tangent;

    explicit VertexPositionNormalUvTangent(
        const glm::vec3 position,
        const glm::vec3 normal,
        const glm::vec2 uv,
        const glm::vec4 tangent)
            : Position(position), Normal(normal), Uv(uv), Tangent(tangent)
    {
    }
};

struct VertexPositionNormalUvw
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Uvw;

    explicit VertexPositionNormalUvw(
        const glm::vec3 position,
        const glm::vec3 normal,
        const glm::vec3 uvw)
            : Position(position), Normal(normal), Uvw(uvw)
    {
    }
};

struct VertexPositionNormalUvwTangent
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Uvw;
    glm::vec4 Tangent;

    explicit VertexPositionNormalUvwTangent(
        const glm::vec3 position,
        const glm::vec3 normal,
        const glm::vec3 uvw,
        const glm::vec4 tangent)
            : Position(position), Normal(normal), Uvw(uvw), Tangent(tangent)
    {
    }
};