#pragma once

#include "graphics/textures.hpp"

#include <glm/glm.hpp>

class Texture;

class Material final
{
public:
    bool operator< (const Material& material) const
    {
        return _textureDiffuse->Id() < material._textureDiffuse->Id() && _textureNormal->Id() < material._textureNormal->Id() && _textureSpecular->Id() < material._textureSpecular->Id();
    }
    
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

    glm::vec3 _diffuse{ 0.2f, 0.2f, 0.2f };
    glm::vec3 _specular{ 0.1f, 0.1f, 0.1f };
};