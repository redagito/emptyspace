#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "physics.hpp"

#include <cmath>

TEST_CASE("Boost applies thrust relative to the camera orientation")
{
	PhysicsScene physics;

	physics.Boost(Direction::Forward, 0.5f);
	REQUIRE(physics.LinearThrust.x == Catch::Approx(0.0f));
	REQUIRE(physics.LinearThrust.y == Catch::Approx(0.0f));
	REQUIRE(physics.LinearThrust.z == Catch::Approx(-0.5f));

	physics.Boost(Direction::Right, 0.5f);
	REQUIRE(physics.LinearThrust.x == Catch::Approx(0.5f));
	REQUIRE(physics.LinearThrust.z == Catch::Approx(-0.5f));
}

TEST_CASE("Stop clears accumulated linear and angular thrust")
{
	PhysicsScene physics;

	physics.Boost(Direction::Forward, 0.5f);
	physics.Tumble(10.0f, 20.0f);
	physics.Boost(Direction::Stop, 0.0f);

	REQUIRE(physics.LinearThrust.x == Catch::Approx(0.0f));
	REQUIRE(physics.LinearThrust.y == Catch::Approx(0.0f));
	REQUIRE(physics.LinearThrust.z == Catch::Approx(0.0f));
	REQUIRE(physics.AngularThrust.x == Catch::Approx(0.0f));
	REQUIRE(physics.AngularThrust.y == Catch::Approx(0.0f));
	REQUIRE(physics.AngularThrust.z == Catch::Approx(0.0f));
}

TEST_CASE("Tumble accumulates angular thrust from mouse movement")
{
	PhysicsScene physics;

	physics.Tumble(10.0f, 20.0f);

	REQUIRE(physics.AngularThrust.x == Catch::Approx(0.02f));
	REQUIRE(physics.AngularThrust.y == Catch::Approx(0.01f));
	REQUIRE(physics.AngularThrust.z == Catch::Approx(0.0f));
}

TEST_CASE("A long frame does not move the simulation far")
{
	PhysicsScene physics;

	physics.Boost(Direction::Forward, 1.0f);
	physics.Step(100.0f);

	const auto position = physics.Camera->getGlobalPose().p;
	const auto distance = std::sqrt(position.x * position.x + position.y * position.y + position.z * position.z);

	REQUIRE(distance < 10.0f);
}
