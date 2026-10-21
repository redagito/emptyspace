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
        Texture* textureSpecular)
        : _textureDiffuse{textureDiffuse},
        _textureNormal{textureNormal},
        _textureSpecular{textureSpecular}
    {
    }

    void Apply() const;

private:
    Texture* _textureDiffuse{};
    Texture* _textureNormal{};
    Texture* _textureSpecular{};
};