#include <emptyspace/graphics/material.hpp>

#include <glad/glad.h>

void Material::Apply() const
{
    glBindTextureUnit(0, _textureDiffuse->Id());
    glBindTextureUnit(1, _textureSpecular->Id());
    glBindTextureUnit(2, _textureNormal->Id());
}