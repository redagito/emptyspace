#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "camera.hpp"
#include "gl_context.hpp"
#include "graphics/graphicsdevice.hpp"
#include "scenes/spacescene.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

TEST_CASE("The ship is drawn in the orientation of the camera")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	SpaceScene scene(*device);
	scene.Initialize();

	Camera camera;
	camera.Position = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.Direction = glm::vec3(0.0f, 0.0f, 1.0f);
	camera.View = glm::mat4(1.0f);
	// A quarter turn around the X axis, so the ship is on its side.
	camera.Orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	scene.Update(0.016f, camera);

	const auto& ship = scene.Objects()[2];
	const auto rotation = glm::mat3(ship->ModelViewProjection);

	// The nose of the ship points along the negative Z axis of the camera.
	const auto nose = glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
	REQUIRE(nose.x == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(nose.y == Catch::Approx(1.0f).margin(0.001f));
	REQUIRE(nose.z == Catch::Approx(0.0f).margin(0.001f));

	// The up of the ship is rolled with the camera, it is not levelled.
	const auto up = glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));
	REQUIRE(up.x == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(up.y == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(up.z == Catch::Approx(1.0f).margin(0.001f));
}

TEST_CASE("The spotlight of the demo scene covers the cubes the camera starts among")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	SpaceScene scene(*device);
	scene.Initialize();

	const Light* spotlight = nullptr;
	for (const auto& light : scene.Lights())
	{
		if (light.Type == LightType::SpotLight)
		{
			spotlight = &light;
		}
	}
	REQUIRE(spotlight != nullptr);

	// The cubes the camera starts among orbit the origin at a radius of 2.
	const glm::vec3 cubePosition{ 2.0f, 0.0f, 0.0f };

	// The cosine of the angle between the direction of the light and the
	// direction from the cube to the light.
	const auto toLight = glm::normalize(spotlight->Position - cubePosition);
	const auto theta = glm::dot(toLight, glm::normalize(-spotlight->Direction));

	// The cut offs are the cosines of the inner and the outer angle of the
	// cone. The cube has to be inside the inner one to be lit fully.
	REQUIRE(theta >= spotlight->CutOff.x);
	REQUIRE(theta <= 1.0f);
}

TEST_CASE("The cubes spin at the same rate whatever the frame rate is")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();

	Camera camera;
	camera.Position = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.Direction = glm::vec3(0.0f, 0.0f, 1.0f);
	camera.View = glm::mat4(1.0f);
	camera.Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	SpaceScene at60(*device);
	at60.Initialize();
	for (int i = 0; i < 60; ++i)
	{
		at60.Update(1.0f / 60.0f, camera);
	}

	SpaceScene at144(*device);
	at144.Initialize();
	for (int i = 0; i < 144; ++i)
	{
		at144.Update(1.0f / 144.0f, camera);
	}

	// After a second the cube in the middle has to be in the same place.
	const auto& slow = at60.Objects()[1]->ModelViewProjection;
	const auto& fast = at144.Objects()[1]->ModelViewProjection;
	for (int column = 0; column < 4; ++column)
	{
		for (int row = 0; row < 4; ++row)
		{
			REQUIRE(slow[column][row] == Catch::Approx(fast[column][row]).margin(0.001f));
		}
	}
}

TEST_CASE("The ship is drawn behind and below the camera")
{
	REQUIRE(GLTestContext() != nullptr);

	auto device = std::make_unique<GraphicsDevice>();
	SpaceScene scene(*device);
	scene.Initialize();

	Camera camera;
	camera.Position = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.Direction = glm::vec3(0.0f, 0.0f, 1.0f);
	camera.View = glm::mat4(1.0f);
	camera.Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	scene.Update(0.016f, camera);

	const auto& ship = scene.Objects()[2];
	const auto position = glm::vec3(ship->ModelViewProjection[3]);

	// Behind the camera, below it and to its right.
	REQUIRE(position.x == Catch::Approx(0.25f).margin(0.001f));
	REQUIRE(position.y == Catch::Approx(-0.5f).margin(0.001f));
	REQUIRE(position.z == Catch::Approx(0.25f).margin(0.001f));
}
