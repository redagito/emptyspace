#pragma once

#include <emptyspace/types.hpp>

#include <vector>

class Texture;

class Framebuffer
{
public:
	Framebuffer(const std::vector<Texture*>& colorAttachments, const Texture* depthAttachment = nullptr);
	~Framebuffer();
	
	void Bind() const;
	void Clear(int colorIndex, const f32* clearColor) const;
	void ClearDepth(const f32 depthClearValue) const;

	[[nodiscard]] u32 Id() const;
private:
	u32 _id{};
};
