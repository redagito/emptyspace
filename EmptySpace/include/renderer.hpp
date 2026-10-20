#pragma once

#include "types.hpp"

#include <glm/mat4x4.hpp>

#include <memory>

struct Camera;
class Framebuffer;
class Frustum;
class Geometry;
class GraphicsDevice;
class Program;
class SpaceScene;
class Texture;
class TextureCube;

struct FrameSize
{
	s32 Width{};
	s32 Height{};
};

class Renderer
{
public:
	explicit Renderer(const FrameSize& renderTarget);
	~Renderer();

	void Render(const Camera& camera, f32 deltaTime, const FrameSize& window);

	void SetTransitionEffectEnabled(const bool enabled) { _isTransitionEffectEnabled = enabled; }
	void SetMotionBlurEnabled(const bool enabled) { _isMotionBlurEnabled = enabled; }

	[[nodiscard]] s32 VisibleLights() const { return _visibleLights; }

private:
	void RenderGBuffer(const glm::mat4& projection, const glm::mat4& view);
	void RenderLights(const glm::mat4& projection, const Camera& camera);
	void RenderEmission();
	void ResolveGBuffer(const Camera& camera, f32 fieldOfView);
	void RenderTransition();
	void RenderMotionBlur();
	void Present(const FrameSize& window);

	FrameSize _renderTarget{};

	std::unique_ptr<GraphicsDevice> _graphicsDevice;
	std::unique_ptr<SpaceScene> _scene;

	std::unique_ptr<Program> _finalProgram;
	std::unique_ptr<Program> _geometryProgram;
	std::unique_ptr<Program> _motionBlurProgram;
	std::unique_ptr<Program> _lightProgram;
	std::unique_ptr<Program> _quadProgram;
	std::unique_ptr<Program> _emissionProgram;

	std::unique_ptr<Geometry> _emptyGeometry;
	std::unique_ptr<Geometry> _cubeGeometry;
	std::unique_ptr<Geometry> _planeGeometry;
	std::unique_ptr<Geometry> _shipGeometry;
	std::unique_ptr<Geometry> _pointLightGeometry;

	std::unique_ptr<Texture> _gBufferFinalTexture;
	std::unique_ptr<Texture> _gBufferPositionTexture;
	std::unique_ptr<Texture> _gBufferNormalTexture;
	std::unique_ptr<Texture> _gBufferAlbedoTexture;
	std::unique_ptr<Texture> _gBufferDepthTexture;
	std::unique_ptr<Texture> _gBufferVelocityTexture;
	std::unique_ptr<Texture> _lightBufferTexture;
	std::unique_ptr<Texture> _motionBlurTexture;
	std::unique_ptr<Texture> _transitionTexture;
	std::unique_ptr<Texture> _emissionTexture;

	std::unique_ptr<Framebuffer> _geometryFramebuffer;
	std::unique_ptr<Framebuffer> _emissionFramebuffer;
	std::unique_ptr<Framebuffer> _finalFramebuffer;
	std::unique_ptr<Framebuffer> _motionBlurFramebuffer;
	std::unique_ptr<Framebuffer> _lightsFramebuffer;
	std::unique_ptr<Framebuffer> _transitionFramebuffer;

	std::unique_ptr<TextureCube> _skyboxTextureCube;

	std::unique_ptr<Frustum> _frustum;

	glm::vec4 _transitionFactor{ 0.0f, 0.0f, 0.0f, 0.0f };

	bool _isMotionBlurEnabled{ true };
	bool _isTransitionEffectEnabled{ false };

	s32 _visibleLights{};
};
