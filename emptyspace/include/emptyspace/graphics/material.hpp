#pragma once
#include <emptyspace/types.hpp>

#include <glad/glad.h>
#include <glm/vec3.hpp>

class Texture;

class Material final
{
public:
	bool operator< (const Material& material) const
	{
		return _textureDiffuse->Id() < material._textureDiffuse->Id() && _textureNormal->Id() < material._textureNormal->Id() && _textureSpecular->Id() < material._textureSpecular->Id();
	}
	
	Material(Texture* textureDiffuse, Texture* textureNormal, Texture* textureSpecular)
		: _textureDiffuse{textureDiffuse}, _textureNormal{textureNormal}, _textureSpecular{textureSpecular}
	{		
	}
	
	void Apply() const
	{
		glBindTextureUnit(0, _textureDiffuse->Id());
		glBindTextureUnit(1, _textureSpecular->Id());
		glBindTextureUnit(2, _textureNormal->Id());
	}

private:
	Texture* _textureDiffuse{};
	Texture* _textureNormal{};
	Texture* _textureSpecular{};

	glm::vec3 _diffuse{ 0.2f, 0.2f, 0.2f };
	glm::vec3 _specular{ 0.1f, 0.1f, 0.1f };
};