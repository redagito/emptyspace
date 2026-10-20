#include "renderer.hpp"

#include "camera.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/geometry.hpp"
#include "graphics/graphicsdevice.hpp"
#include "graphics/light.hpp"
#include "graphics/material.hpp"
#include "graphics/program.hpp"
#include "graphics/texturecube.hpp"
#include "graphics/textures.hpp"
#include "math/frustum.hpp"
#include "scenes/scene.hpp"
#include "scenes/spacescene.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string_view>

namespace
{
	constexpr auto kUniformProjectionMatrix = 0;
	constexpr auto kUniformViewMatrix = 1;
	constexpr auto kUniformModelMatrix = 2;

	constexpr auto kUniformModelViewProjectionCurrent = 3;
	constexpr auto kUniformModelViewProjectionPrevious = 4;
	constexpr auto kUniformExcludeFromMotionBlur = 5;
	constexpr auto kUniformIsInstanced = 6;

	constexpr auto kUniformLightType = 0;
	constexpr auto kUniformLightPosition = 1;
	constexpr auto kUniformLightColor = 2;
	constexpr auto kUniformLightDirection = 3;
	constexpr auto kUniformLightAttenuation = 4;
	constexpr auto kUniformLightCutOff = 5;
	constexpr auto kUniformCameraPosition = 6;

	constexpr auto kUniformCameraDirection = 0;
	constexpr auto kUniformCameraFieldOfView = 1;
	constexpr auto kUniformCameraAspectRatio = 2;
	constexpr auto kUniformUvsDiff = 3;

	constexpr auto kUniformVelocityScale = 0;
	constexpr auto kUniformEmissionThreshold = 0;
	constexpr auto kUniformBlendColor = 0;

	constexpr auto kFieldOfView = glm::radians(60.0f);
	constexpr auto kAsteroidInstanceCount = 5000;
	constexpr auto kPointLightVertexCount = 240;
}

Renderer::Renderer(const FrameSize& renderTarget)
	: _renderTarget{ renderTarget }
{
	_graphicsDevice = std::make_unique<GraphicsDevice>();
	_scene = std::make_unique<SpaceScene>(*_graphicsDevice);

	_gBufferFinalTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_gBufferPositionTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGBA16F, GL_RGB, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_gBufferNormalTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_gBufferAlbedoTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGBA8, GL_RGBA, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_gBufferDepthTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_DEPTH_COMPONENT32, GL_DEPTH, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_gBufferVelocityTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RG16F, GL_RG, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_lightBufferTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_motionBlurTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_transitionTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));
	_emissionTexture = std::unique_ptr<Texture>(_graphicsDevice->CreateTexture(GL_RGBA16F, GL_RGBA, _renderTarget.Width, _renderTarget.Height, nullptr, GL_NEAREST));

	_geometryFramebuffer = std::unique_ptr<Framebuffer>(_graphicsDevice->CreateFramebuffer("FB_Geometry",
		{
			_gBufferPositionTexture.get(),
			_gBufferNormalTexture.get(),
			_gBufferAlbedoTexture.get(),
			_gBufferVelocityTexture.get(),
			_emissionTexture.get(),
		},
		_gBufferDepthTexture.get()));
	_finalFramebuffer = std::unique_ptr<Framebuffer>(_graphicsDevice->CreateFramebuffer("FB_Final", { _gBufferFinalTexture.get() }));
	_emissionFramebuffer = std::unique_ptr<Framebuffer>(_graphicsDevice->CreateFramebuffer("FB_Emission", { _emissionTexture.get() }));
	_motionBlurFramebuffer = std::unique_ptr<Framebuffer>(_graphicsDevice->CreateFramebuffer("FB_Motionblur", { _motionBlurTexture.get() }));
	_lightsFramebuffer = std::unique_ptr<Framebuffer>(_graphicsDevice->CreateFramebuffer("FB_Lights", { _lightBufferTexture.get() }));
	_transitionFramebuffer = std::unique_ptr<Framebuffer>(_graphicsDevice->CreateFramebuffer("FB_Transition", { _transitionTexture.get() }));

	_skyboxTextureCube = std::unique_ptr<TextureCube>(_graphicsDevice->CreateTextureCubeFromFiles({
		"data/textures/TC_SkySpace_Xn.png",
		"data/textures/TC_SkySpace_Xp.png",
		"data/textures/TC_SkySpace_Yn.png",
		"data/textures/TC_SkySpace_Yp.png",
		"data/textures/TC_SkySpace_Zn.png",
		"data/textures/TC_SkySpace_Zp.png"
		}));

	_emptyGeometry = std::unique_ptr<Geometry>(Geometry::CreateEmpty());
	_cubeGeometry = std::unique_ptr<Geometry>(Geometry::CreateUnitCube());
	_planeGeometry = std::unique_ptr<Geometry>(Geometry::CreateUnitPlane());
	_shipGeometry = std::unique_ptr<Geometry>(Geometry::CreateFromFile("data/models/SM_ShipA_noWindshield.obj"));
	_pointLightGeometry = std::unique_ptr<Geometry>(Geometry::CreateFromFile("data/models/SM_PointLight.obj"));

	_finalProgram = std::unique_ptr<Program>(_graphicsDevice->CreateProgramFromFiles(
		"PP_Final",
		"data/shaders/main.vert.glsl",
		"data/shaders/main.frag.glsl"));
	_geometryProgram = std::unique_ptr<Program>(_graphicsDevice->CreateProgramFromFiles(
		"PP_Geometry",
		"data/shaders/gbuffer.vert.glsl",
		"data/shaders/gbuffer.frag.glsl"));
	_motionBlurProgram = std::unique_ptr<Program>(_graphicsDevice->CreateProgramFromFiles(
		"PP_MotionBlur",
		"data/shaders/motionblur.vert.glsl",
		"data/shaders/motionblur.frag.glsl"));
	_lightProgram = std::unique_ptr<Program>(_graphicsDevice->CreateProgramFromFiles(
		"PP_Light",
		"data/shaders/light.vert.glsl",
		"data/shaders/light.frag.glsl"));
	_quadProgram = std::unique_ptr<Program>(_graphicsDevice->CreateProgramFromFiles(
		"PP_FSQ",
		"data/shaders/quad.vert.glsl",
		"data/shaders/quad.frag.glsl"));
	_emissionProgram = std::unique_ptr<Program>(_graphicsDevice->CreateProgramFromFiles(
		"PP_Emission",
		"data/shaders/emission.vert.glsl",
		"data/shaders/emission.frag.glsl"));

	_frustum = std::make_unique<Frustum>();

	_scene->Initialize();
}

Renderer::~Renderer() = default;

void Renderer::Render(const Camera& camera, const f32 deltaTime, const FrameSize& window)
{
	if (_isTransitionEffectEnabled)
	{
		_transitionFactor.w += 0.01f;
		if (_transitionFactor.w >= 1.0f)
		{
			_isTransitionEffectEnabled = false;
			_transitionFactor.w = 0.0f;
		}
	}

	_scene->Update(deltaTime, camera);

	const auto projection = glm::perspective(
		kFieldOfView,
		static_cast<f32>(_renderTarget.Width) / static_cast<f32>(_renderTarget.Height),
		0.1f,
		1000.0f);

	_frustum->CalculateFrustum(projection, camera.View);

	RenderGBuffer(projection, camera.View);
	RenderLights(projection, camera);
	RenderEmission();
	ResolveGBuffer(camera, kFieldOfView);

	if (_transitionFactor.w > 0.0f)
	{
		RenderTransition();
	}

	if (_isMotionBlurEnabled)
	{
		RenderMotionBlur();
	}

	Present(window);
}

void Renderer::RenderGBuffer(const glm::mat4& projection, const glm::mat4& view)
{
	constexpr std::string_view renderGBufferDebugGroup = "Render GBuffer";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, static_cast<GLsizei>(renderGBufferDebugGroup.length()), renderGBufferDebugGroup.data());

	auto constexpr depthClearValue = 1.0f;
	_geometryFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
	_geometryFramebuffer->Clear(1, glm::value_ptr(glm::vec3(0.0f)));
	_geometryFramebuffer->Clear(2, glm::value_ptr(glm::vec4(0.0f)));
	_geometryFramebuffer->Clear(3, glm::value_ptr(glm::vec2(0.0f)));
	_geometryFramebuffer->Clear(4, glm::value_ptr(glm::vec4(0.0f)));
	_geometryFramebuffer->ClearDepth(depthClearValue);
	_geometryFramebuffer->Bind();
	glViewport(0, 0, _renderTarget.Width, _renderTarget.Height);

	_geometryProgram->Bind();
	_geometryProgram->SetVertexShaderUniform(kUniformProjectionMatrix, projection);
	_geometryProgram->SetVertexShaderUniform(kUniformViewMatrix, view);

	for (auto& object : _scene->Objects())
	{
		object->ObjectMaterial->Apply();
		switch (object->ObjectShape)
		{
			case Shape::Cube: _cubeGeometry->Bind(); break;
			case Shape::CubeInstanced:
			{
				_cubeGeometry->Bind();
				_scene->GetAsteroidInstanceBuffer()->BindAsStorageBuffer(0);
				object->ExcludeFromMotionBlur = true;
				break;
			}
			case Shape::Ship: _shipGeometry->Bind(); break;
			case Shape::Quad: _planeGeometry->Bind(); break;
		}

		auto const currentModelViewProjection = projection * view * object->ModelViewProjection;

		_geometryProgram->SetVertexShaderUniform(kUniformModelMatrix, object->ModelViewProjection);
		_geometryProgram->SetVertexShaderUniform(kUniformModelViewProjectionCurrent, currentModelViewProjection);
		_geometryProgram->SetVertexShaderUniform(kUniformModelViewProjectionPrevious, object->ModelViewProjectionPrevious);
		_geometryProgram->SetVertexShaderUniform(kUniformExcludeFromMotionBlur, object->ExcludeFromMotionBlur);
		_geometryProgram->SetVertexShaderUniform(kUniformIsInstanced, object->ObjectShape == Shape::CubeInstanced);

		object->ModelViewProjectionPrevious = currentModelViewProjection;

		switch (object->ObjectShape)
		{
			case Shape::Cube: _cubeGeometry->Draw(); break;
			case Shape::CubeInstanced: _cubeGeometry->DrawInstanced(kAsteroidInstanceCount); break;
			case Shape::Quad: _planeGeometry->Draw(); break;
			case Shape::Ship: _shipGeometry->Draw(); break;
		}
	}
	glPopDebugGroup();
}

void Renderer::RenderLights(const glm::mat4& projection, const Camera& camera)
{
	constexpr std::string_view renderLightBufferDebugGroup = "Render LBuffer";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, static_cast<GLsizei>(renderLightBufferDebugGroup.length()), renderLightBufferDebugGroup.data());

	auto constexpr depthClearValue = 1.0f;
	_lightsFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
	_lightsFramebuffer->ClearDepth(depthClearValue);
	_lightsFramebuffer->Bind();

	_gBufferPositionTexture->Bind(0);
	_gBufferNormalTexture->Bind(1);
	_gBufferDepthTexture->Bind(2);

	_lightProgram->Bind();
	_pointLightGeometry->Bind();

	glCullFace(GL_FRONT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	_visibleLights = 0;

	for (auto& light : _scene->Lights())
	{
		if (!_frustum->SphereInFrustum(light.Position.x, light.Position.y, light.Position.z, light.Attenuation.z))
		{
			continue;
		}

		_visibleLights++;

		auto model = glm::translate(glm::mat4(1.0f), glm::vec3(light.Position));
		model = glm::scale(model, glm::vec3(light.Attenuation.z, light.Attenuation.z, light.Attenuation.z));

		_lightProgram->SetVertexShaderUniform(kUniformProjectionMatrix, projection);
		_lightProgram->SetVertexShaderUniform(kUniformViewMatrix, camera.View);
		_lightProgram->SetVertexShaderUniform(kUniformModelMatrix, model);

		_lightProgram->SetFragmentShaderUniform(kUniformLightType, static_cast<s32>(light.Type));
		_lightProgram->SetFragmentShaderUniform(kUniformLightPosition, light.Position);
		_lightProgram->SetFragmentShaderUniform(kUniformLightColor, light.Color);
		_lightProgram->SetFragmentShaderUniform(kUniformLightDirection, light.Direction);
		_lightProgram->SetFragmentShaderUniform(kUniformLightAttenuation, light.Attenuation);
		_lightProgram->SetFragmentShaderUniform(kUniformLightCutOff, light.CutOff);
		_lightProgram->SetFragmentShaderUniform(kUniformCameraPosition, camera.Position);

		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, kPointLightVertexCount, 1, 0);
	}
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);
	glPopDebugGroup();
}

void Renderer::RenderEmission()
{
	constexpr std::string_view renderEmissionDebugGroup = "Render Emission";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 4, static_cast<GLsizei>(renderEmissionDebugGroup.length()), renderEmissionDebugGroup.data());

	_emissionFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
	_emissionFramebuffer->Bind();

	_lightBufferTexture->Bind(0);
	_emissionTexture->Bind(1);

	_emptyGeometry->Bind();
	_emissionProgram->Bind();
	_emissionProgram->SetFragmentShaderUniform(kUniformEmissionThreshold, 0.7f);

	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
	glPopDebugGroup();
}

void Renderer::ResolveGBuffer(const Camera& camera, const f32 fieldOfView)
{
	constexpr std::string_view resolveGBufferDebugGroup = "Resolve GBuffer";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 3, static_cast<GLsizei>(resolveGBufferDebugGroup.length()), resolveGBufferDebugGroup.data());

	_finalFramebuffer->Clear(0, glm::value_ptr(glm::vec3(1.0f)));
	_finalFramebuffer->ClearDepth(1.0f);
	_finalFramebuffer->Bind();

	_gBufferPositionTexture->Bind(0);
	_gBufferNormalTexture->Bind(1);
	_gBufferAlbedoTexture->Bind(2);
	_gBufferDepthTexture->Bind(3);
	_skyboxTextureCube->Bind(4);
	_lightBufferTexture->Bind(5);
	_emissionTexture->Bind(6);

	_emptyGeometry->Bind();
	_finalProgram->Bind();
	_finalProgram->SetVertexShaderUniform(kUniformCameraDirection, glm::inverse(glm::mat3(camera.View)));
	_finalProgram->SetVertexShaderUniform(kUniformCameraFieldOfView, fieldOfView);
	_finalProgram->SetVertexShaderUniform(kUniformCameraAspectRatio, static_cast<f32>(_renderTarget.Width) / static_cast<f32>(_renderTarget.Height));
	_finalProgram->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(1.0f, 1.0f));

	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
	glPopDebugGroup();
}

void Renderer::RenderTransition()
{
	constexpr std::string_view transitionDebugGroup = "Transition";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 5, static_cast<GLsizei>(transitionDebugGroup.length()), transitionDebugGroup.data());

	_transitionFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0)));
	_transitionFramebuffer->Bind();

	_gBufferFinalTexture->Bind(0);

	_quadProgram->Bind();
	_quadProgram->SetFragmentShaderUniform(kUniformBlendColor, _transitionFactor);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glPopDebugGroup();
}

void Renderer::RenderMotionBlur()
{
	constexpr std::string_view motionBlurDebugGroup = "MotionBlur";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 6, static_cast<GLsizei>(motionBlurDebugGroup.length()), motionBlurDebugGroup.data());

	_motionBlurFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
	_motionBlurFramebuffer->Bind();

	if (_transitionFactor.w > 0.0f)
	{
		_transitionTexture->Bind(0);
	}
	else
	{
		_gBufferFinalTexture->Bind(0);
	}
	_gBufferVelocityTexture->Bind(1);

	_emptyGeometry->Bind();
	_motionBlurProgram->Bind();
	_motionBlurProgram->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(1.0f, 1.0f));
	_motionBlurProgram->SetFragmentShaderUniform(kUniformVelocityScale, 2.0f);

	glCullFace(GL_FRONT);
	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
	glCullFace(GL_BACK);
	glPopDebugGroup();
}

void Renderer::Present(const FrameSize& window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window.Width, window.Height);
	glBlitNamedFramebuffer(_isMotionBlurEnabled
		? _motionBlurFramebuffer->Id()
		: _isTransitionEffectEnabled
			? _transitionFramebuffer->Id()
			: _finalFramebuffer->Id(), 0, 0, 0, _renderTarget.Width, _renderTarget.Height, 0, 0, window.Width, window.Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
