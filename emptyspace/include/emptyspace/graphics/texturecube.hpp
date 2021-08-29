#pragma once

#include <emptyspace/types.hpp>
#include <emptyspace/graphics/textures.hpp>

#include <array>

class TextureCube
{
public:
    static TextureCube* FromFiles(const std::array<std::string_view, 6>& filePaths, u32 comp = STBI_rgb_alpha);

    TextureCube(const u32 internalFormat, const u32 format, const s32 width, const s32 height, std::array<stbi_uc*, 6> const& data);
    ~TextureCube();

    [[nodiscard]] u32 Id() const;
    void Bind(const u32 textureUnit) const;
private:
    u32 _id{};
};
