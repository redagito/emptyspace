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

	glm::vec4 PixelAt(const std::vector<f32>& data, const s32 x, const s32 y)
	{
		const auto offset = (static_cast<std::size_t>(y) * kSize + x) * 4;
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
	auto scene = std::make_unique<TestScene>(*device, glm::vec3(0.0f, 0.0f, -10.0f));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto gbuffer = ReadTexture(renderer.GBufferPosition(), kSize, kSize);
	const auto centre = PixelAt(gbuffer, kSize / 2, kSize / 2);

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
	auto scene = std::make_unique<TestScene>(*device, glm::vec3(0.0f, 0.0f, -10.0f));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto gbuffer = ReadTexture(renderer.GBufferNormal(), kSize, kSize);
	const auto centre = PixelAt(gbuffer, kSize / 2, kSize / 2);

	REQUIRE(std::isfinite(centre.x));
	REQUIRE(std::isfinite(centre.y));
	REQUIRE(std::isfinite(centre.z));

	// The face towards the camera points along +Z.
	REQUIRE(centre.x == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(centre.y == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(centre.z == Catch::Approx(1.0f).margin(0.05f));
}

TEST_CASE("A point light lights a surface in front of it")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, glm::vec3(0.0f, 0.0f, -10.0f));
	scene->AddLight(LightType::PointLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f);

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto lights = ReadTexture(renderer.LightBuffer(), kSize, kSize);
	const auto centre = PixelAt(lights, kSize / 2, kSize / 2);

	REQUIRE(centre.r > 0.0f);
	REQUIRE(centre.g > 0.0f);
	REQUIRE(centre.b > 0.0f);
}

TEST_CASE("A spot light only lights surfaces inside its cone")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	auto scene = std::make_unique<TestScene>(*device, glm::vec3(0.0f, 0.0f, -10.0f));
	scene->AddLight(LightType::SpotLight, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20.0f,
		glm::vec3(0.0f, 0.0f, 1.0f));

	Renderer renderer(FrameSize{ kSize, kSize }, std::move(scene));
	renderer.Render(CameraAtOrigin(), 0.016f, FrameSize{ kSize, kSize });

	const auto lights = ReadTexture(renderer.LightBuffer(), kSize, kSize);
	const auto centre = PixelAt(lights, kSize / 2, kSize / 2);

	// The cone points away from the surface, so the surface stays dark.
	REQUIRE(centre.r == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(centre.g == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(centre.b == Catch::Approx(0.0f).margin(0.001f));
}
