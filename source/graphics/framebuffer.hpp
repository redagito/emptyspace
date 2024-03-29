#pragma once

#include "types.hpp"

#include <vector>
#include <string_view>

class Texture;

class Framebuffer
{
public:
    Framebuffer(const std::string_view label, const std::vector<Texture*>& colorAttachments, const Texture* depthAttachment = nullptr);
    ~Framebuffer();
    
    void Bind() const;
    void Clear(const int colorIndex, const f32* clearColor) const;
    void ClearDepth(const f32 depthClearValue) const;

    [[nodiscard]] u32 Id() const;
private:
    u32 _id{};
};
