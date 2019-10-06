#pragma once
#include <emptyspace/types.hpp>

#include <glad/glad.h>
#include <glm/vec3.hpp>

class Material final
{
public:
	bool operator< (const Material& material) const
	{
		return _textureDiffuse < material._textureDiffuse && _textureNormal < material._textureNormal && _textureSpecular < material._textureSpecular;
	}
	
	Material(const u32 textureDiffuse, const u32 textureNormal, const u32 textureSpecular)
		: _textureDiffuse{textureDiffuse}, _textureNormal{textureNormal}, _textureSpecular{textureSpecular}
	{		
	}
	
	void Apply() const
	{
		glBindTextureUnit(0, _textureDiffuse);
		glBindTextureUnit(1, _textureSpecular);
		glBindTextureUnit(2, _textureNormal);
	}

private:
	u32 _textureDiffuse{};
	u32 _textureNormal{};
	u32 _textureSpecular{};

	glm::vec3 _diffuse{ 0.2f, 0.2f, 0.2f };
	glm::vec3 _specular{ 0.1f, 0.1f, 0.1f };
};