#pragma once

#include "graphics/textures.hpp"

#include <glm/glm.hpp>

class Texture;

class Material final
{
public:
    Material(
        Texture* textureDiffuse,
        Texture* textureNormal,
        Texture* textureSpecular,
        glm::vec3 emission = glm::vec3(0.0f))
        : _textureDiffuse{textureDiffuse},
        _textureNormal{textureNormal},
        _textureSpecular{textureSpecular},
        _emission{emission}
    {
    }

    void Apply() const;

    [[nodiscard]] const glm::vec3& Emission() const { return _emission; }

private:
    Texture* _textureDiffuse{};
    Texture* _textureNormal{};
    Texture* _textureSpecular{};
    glm::vec3 _emission{};
};