#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "gl_context.hpp"
#include "renderer.hpp"
#include "test_support.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <memory>

namespace
{
	constexpr s32 kSize = 64;

	glm::vec4 PixelAt(const std::vector<f32>& data, const s32 width, const s32 x, const s32 y)
	{
		const auto offset = (static_cast<std::size_t>(y) * width + x) * 4;
		return glm::vec4(data[offset], data[offset + 1], data[offset + 2], data[offset + 3]);
	}

	Camera CameraAtOrigin()
	{
		Camera camera;
		camera.Position = glm::vec3(0.0f);
		camera.Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		camera.View = glm::mat4(1.0f);
		return camera;
	}
}

TEST_CASE("The G buffer stores the world position of a fragment")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto gbuffer = ReadTexture(renderer.GBufferPosition(), kSize, kSize);
	const auto centre = PixelAt(gbuffer, kSize, kSize / 2, kSize / 2);

	// The unit cube sits half a unit around its origin, so the face towards
	// the camera is at (0, 0, -9.5). The middle pixel of an even sized target
	// is half a pixel off the axis.
	REQUIRE(centre.x == Catch::Approx(0.0f).margin(0.25f));
	REQUIRE(centre.y == Catch::Approx(0.0f).margin(0.25f));
	REQUIRE(centre.z == Catch::Approx(-9.5f).margin(0.01f));
}

TEST_CASE("The G buffer normal of a face is the normal of that face")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto gbuffer = ReadTexture(renderer.GBufferNormal(), kSize, kSize);
	const auto centre = PixelAt(gbuffer, kSize, kSize / 2, kSize / 2);

	REQUIRE(std::isfinite(centre.x));
	REQUIRE(std::isfinite(centre.y));
	REQUIRE(std::isfinite(centre.z));

	// The face towards the camera points along +Z.
	REQUIRE(centre.x == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(centre.y == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(centre.z == Catch::Approx(1.0f).margin(0.05f));
}

TEST_CASE("Geometry without a tangent keeps its vertex normal")
{
	REQUIRE(GLTestContext() != nullptr);

	// A normal map that points away from the surface. Without a tangent
	// there is no tangent space to map it into, so the vertex normal is used.
	const u8 normalMap[4] = { 128, 128, 0, 255 };

	// The unit plane has no tangent attribute, so the normal of the face has
	// to come from the vertex normal.
	const auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Quad, model, normalMap);

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto gbuffer = ReadTexture(renderer.GBufferNormal(), kSize, kSize);
	const auto centre = PixelAt(gbuffer, kSize, kSize / 2, kSize / 2);

	REQUIRE(std::isfinite(centre.x));
	REQUIRE(std::isfinite(centre.y));
	REQUIRE(std::isfinite(centre.z));

	// The plane points along +Y, rotated by 45 degrees around the X axis.
	REQUIRE(centre.x == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(centre.y == Catch::Approx(0.707f).margin(0.05f));
	REQUIRE(centre.z == Catch::Approx(0.707f).margin(0.05f));
}

TEST_CASE("The emission pass fills the emission buffer")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)), nullptr, nullptr,
		glm::vec3(0.25f, 0.5f, 0.75f));
	scene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 12.0f);

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto emission = ReadTexture(renderer.Emission(), kSize, kSize);
	const auto centre = PixelAt(emission, kSize, kSize / 2, kSize / 2);

	REQUIRE(centre.r > 0.0f);
	REQUIRE(centre.g > 0.0f);
	REQUIRE(centre.b > 0.0f);
}

TEST_CASE("The resolved image contains the emission")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)), nullptr, nullptr,
		glm::vec3(0.25f, 0.5f, 0.75f));
	scene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 12.0f);

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto lights = ReadTexture(renderer.LightBuffer(), kSize, kSize);
	const auto lightCentre = PixelAt(lights, kSize, kSize / 2, kSize / 2);

	const auto final = ReadTexture(renderer.Final(), kSize, kSize);
	const auto finalCentre = PixelAt(final, kSize, kSize / 2, kSize / 2);

	REQUIRE(lightCentre.r > 0.0f);
	REQUIRE(finalCentre.r > lightCentre.r + 0.1f);
}

TEST_CASE("The renderer follows a resize")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	REQUIRE(renderer.GBufferPosition().Width() == kSize);
	REQUIRE(renderer.GBufferPosition().Height() == kSize);

	renderer.Resize(kSize / 2, kSize / 2);
	REQUIRE(renderer.GBufferPosition().Width() == kSize / 2);
	REQUIRE(renderer.GBufferPosition().Height() == kSize / 2);

	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize / 2, kSize / 2 });

	const auto gbuffer = ReadTexture(renderer.GBufferPosition(), kSize / 2, kSize / 2);
	const auto centre = PixelAt(gbuffer, kSize / 2, kSize / 4, kSize / 4);

	REQUIRE(centre.z == Catch::Approx(-9.5f).margin(0.01f));
}

TEST_CASE("A surface without an emissive material does not emit")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto emission = ReadTexture(renderer.GBufferEmission(), kSize, kSize);
	const auto centre = PixelAt(emission, kSize, kSize / 2, kSize / 2);

	REQUIRE(centre.r == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(centre.g == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(centre.b == Catch::Approx(0.0f).margin(0.001f));
}

TEST_CASE("An emissive material emits its colour")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)), nullptr, nullptr,
		glm::vec3(0.25f, 0.5f, 0.75f));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto emission = ReadTexture(renderer.GBufferEmission(), kSize, kSize);
	const auto centre = PixelAt(emission, kSize, kSize / 2, kSize / 2);

	REQUIRE(centre.r == Catch::Approx(0.25f).margin(0.01f));
	REQUIRE(centre.g == Catch::Approx(0.5f).margin(0.01f));
	REQUIRE(centre.b == Catch::Approx(0.75f).margin(0.01f));
}

TEST_CASE("The tangent of a face follows its texture axis")
{
	REQUIRE(GLTestContext() != nullptr);

	// A normal map that points along the u axis of the tangent space.
	const u8 tangentNormal[4] = { 255, 128, 128, 255 };

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)), tangentNormal);

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto gbuffer = ReadTexture(renderer.GBufferNormal(), kSize, kSize);
	const auto centre = PixelAt(gbuffer, kSize, kSize / 2, kSize / 2);

	// The face towards the camera has its u axis along +X, so the mapped
	// normal ends up along the tangent.
	REQUIRE(centre.x == Catch::Approx(1.0f).margin(0.05f));
	REQUIRE(centre.y == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(centre.z == Catch::Approx(0.0f).margin(0.05f));
}

TEST_CASE("The specular map of the material scales the highlight")
{
	REQUIRE(GLTestContext() != nullptr);

	const u8 whiteSpecular[4] = { 255, 255, 255, 255 };

	auto blackDevice = std::make_unique<GraphicsDevice>();
	auto blackScene = std::make_unique<TestScene>(*blackDevice, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));
	blackScene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f);

	Renderer blackRenderer(FrameSize{ kSize, kSize }, std::move(blackScene));
	blackRenderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });
	const auto blackLights = ReadTexture(blackRenderer.LightBuffer(), kSize, kSize);
	const auto blackCentre = PixelAt(blackLights, kSize, kSize / 2, kSize / 2);

	auto whiteDevice = std::make_unique<GraphicsDevice>();
	auto whiteScene = std::make_unique<TestScene>(*whiteDevice, Shape::Cube,
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)), nullptr, whiteSpecular);
	whiteScene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f);

	Renderer whiteRenderer(FrameSize{ kSize, kSize }, std::move(whiteScene));
	whiteRenderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });
	const auto whiteLights = ReadTexture(whiteRenderer.LightBuffer(), kSize, kSize);
	const auto whiteCentre = PixelAt(whiteLights, kSize, kSize / 2, kSize / 2);

	// A black specular map leaves only the diffuse, a white one adds the
	// highlight of the same size again.
	REQUIRE(blackCentre.r == Catch::Approx(0.525f).margin(0.01f));
	REQUIRE(whiteCentre.r == Catch::Approx(1.05f).margin(0.01f));
}

TEST_CASE("The transition takes the same time whatever the frame rate is")
{
	REQUIRE(GLTestContext() != nullptr);

	const auto render = [](const int frames, const f32 deltaTime)
	{
		auto device = std::make_unique<GraphicsDevice>();
		auto scene = std::make_unique<TestScene>(*device, Shape::Cube,
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));
		// The surface has to be lit, otherwise the transition fades black to
		// black and there is nothing to compare.
		scene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f);

		Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
		renderer.SetTransitionEffectEnabled(true);
		for (int i = 0; i < frames; ++i)
		{
			renderer.Render(CameraAtOrigin(), deltaTime, FrameSize{ kSize, kSize });
		}

		return ReadTexture(renderer.Final(), kSize, kSize);
	};

	// Half a second of transition at 30 and at 60 frames per second has to
	// fade the image by the same amount.
	const auto at30 = render(15, 1.0f / 30.0f);
	const auto at60 = render(30, 1.0f / 60.0f);

	REQUIRE(at30 == at60);
}

TEST_CASE("A point light lights a surface in front of it")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));
	scene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f);

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto lights = ReadTexture(renderer.LightBuffer(), kSize, kSize);
	const auto centre = PixelAt(lights, kSize, kSize / 2, kSize / 2);

	REQUIRE(centre.r > 0.0f);
	REQUIRE(centre.g > 0.0f);
	REQUIRE(centre.b > 0.0f);
}

TEST_CASE("A spot light only lights surfaces inside its cone")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, Shape::Cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)));
	scene->AddLight(LightType::SpotLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f,
		glm::vec3(0.0f, 0.0f, 1.0f));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto lights = ReadTexture(renderer.LightBuffer(), kSize, kSize);
	const auto centre = PixelAt(lights, kSize, kSize / 2, kSize / 2);

	// The cone points away from the surface, so the surface stays dark.
	REQUIRE(centre.r == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(centre.g == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(centre.b == Catch::Approx(0.0f).margin(0.001f));
}
