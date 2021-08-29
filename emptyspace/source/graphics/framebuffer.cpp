#include <emptyspace/graphics/framebuffer.hpp>
#include <emptyspace/graphics/textures.hpp>

#include <glad/glad.h>
#include <array>
#include <exception>
#include <stdexcept>

Framebuffer::Framebuffer(const std::string_view label, const std::vector<Texture*>& colorAttachments, const Texture* depthAttachment)
{
    glCreateFramebuffers(1, &_id);

    for (std::size_t i = 0; i < colorAttachments.size(); i++)
    {
        glNamedFramebufferTexture(_id, u32(GL_COLOR_ATTACHMENT0 + i), colorAttachments[i]->Id(), 0);
    }

    std::array<u32, 32> drawBuffers{};
    for (std::size_t i = 0; i < colorAttachments.size(); i++)
    {
        drawBuffers[i] = u32(GL_COLOR_ATTACHMENT0 + i);
    }

    glNamedFramebufferDrawBuffers(_id, u32(colorAttachments.size()), drawBuffers.data());

    if (depthAttachment != nullptr)
    {
        glNamedFramebufferTexture(_id, GL_DEPTH_ATTACHMENT, depthAttachment->Id(), 0);
    }

    if (glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("GL: Incomplete framebuffer.");
    }

    glObjectLabel(GL_FRAMEBUFFER, _id, static_cast<GLsizei>(label.length()), label.data());
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &_id);
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void Framebuffer::Clear(const int colorIndex, const f32* clearColor) const
{
    glClearNamedFramebufferfv(_id, GL_COLOR, colorIndex, clearColor);
}

void Framebuffer::ClearDepth(const f32 depthClearValue) const
{
    glClearNamedFramebufferfv(_id, GL_DEPTH, 0, &depthClearValue);
}

u32 Framebuffer::Id() const
{
    return _id;
}
