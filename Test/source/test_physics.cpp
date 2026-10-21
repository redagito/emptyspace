#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "physics.hpp"

#include <cmath>
#include <glm/gtc/quaternion.hpp>

namespace
{
	glm::vec3 LinearVelocity(const PhysicsScene& physics)
	{
		const auto velocity = physics.Camera->getLinearVelocity();
		return glm::vec3(velocity.x, velocity.y, velocity.z);
	}

	glm::vec3 AngularVelocity(const PhysicsScene& physics)
	{
		const auto velocity = physics.Camera->getAngularVelocity();
		return glm::vec3(velocity.x, velocity.y, velocity.z);
	}

	// Turns the ship a quarter turn around the Y axis, so its X axis points
	// along the negative Z axis of the world.
	void YawQuarterTurn(PhysicsScene& physics)
	{
		auto pose = physics.Camera->getGlobalPose();
		// PhysX orders the components of a quaternion as x, y, z, w.
		pose.q = physx::PxQuat(0.0f, 0.70710678f, 0.0f, 0.70710678f);
		physics.Camera->setGlobalPose(pose);
		physics.Camera->setLinearVelocity(physx::PxVec3(0.0f));
		physics.Camera->setAngularVelocity(physx::PxVec3(0.0f));
	}
}

TEST_CASE("The ship reaches the speed it is commanded with")
{
	PhysicsScene physics;

	physics.SetTargetSpeed(glm::vec3(0.0f, 0.0f, -10.0f));
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}

	const auto velocity = LinearVelocity(physics);
	REQUIRE(velocity.x == Catch::Approx(0.0f).margin(0.1f));
	REQUIRE(velocity.y == Catch::Approx(0.0f).margin(0.1f));
	REQUIRE(velocity.z == Catch::Approx(-10.0f).margin(0.5f));
}

TEST_CASE("The ship brakes when it is commanded to stop")
{
	PhysicsScene physics;

	physics.SetTargetSpeed(glm::vec3(0.0f, 0.0f, -10.0f));
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}
	REQUIRE(std::abs(LinearVelocity(physics).z) > 5.0f);

	physics.Stop();
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}

	const auto velocity = LinearVelocity(physics);
	REQUIRE(velocity.x == Catch::Approx(0.0f).margin(0.1f));
	REQUIRE(velocity.y == Catch::Approx(0.0f).margin(0.1f));
	REQUIRE(velocity.z == Catch::Approx(0.0f).margin(0.1f));
}

TEST_CASE("The speed is commanded in the frame of the ship")
{
	PhysicsScene physics;
	YawQuarterTurn(physics);

	// The nose of the ship points along the negative X axis of the world now.
	physics.SetTargetSpeed(glm::vec3(0.0f, 0.0f, -10.0f));
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}

	const auto velocity = LinearVelocity(physics);
	REQUIRE(velocity.x == Catch::Approx(-10.0f).margin(0.5f));
	REQUIRE(velocity.y == Catch::Approx(0.0f).margin(0.1f));
	REQUIRE(velocity.z == Catch::Approx(0.0f).margin(0.1f));
}

TEST_CASE("The turn rate is applied about the axes of the ship")
{
	PhysicsScene physics;
	YawQuarterTurn(physics);

	// A pitch is a turn around the X axis of the ship, which points along the
	// negative Z axis of the world now.
	physics.SetTargetTurnRate(glm::vec3(1.0f, 0.0f, 0.0f));
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}

	const auto angularVelocity = AngularVelocity(physics);
	REQUIRE(angularVelocity.x == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(angularVelocity.y == Catch::Approx(0.0f).margin(0.05f));
	REQUIRE(angularVelocity.z < -0.2f);
}

TEST_CASE("The ship stops turning when the turn rate is released")
{
	PhysicsScene physics;

	physics.SetTargetTurnRate(glm::vec3(0.0f, 1.0f, 0.0f));
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}
	REQUIRE(std::abs(AngularVelocity(physics).y) > 0.2f);

	physics.SetTargetTurnRate(glm::vec3(0.0f));
	for (int i = 0; i < 60; ++i)
	{
		physics.Step(1.0f / 60.0f);
	}

	REQUIRE(std::abs(AngularVelocity(physics).y) < 0.05f);
}

TEST_CASE("A long frame does not move the simulation far")
{
	PhysicsScene physics;

	physics.SetTargetSpeed(glm::vec3(0.0f, 0.0f, -10.0f));
	physics.Step(100.0f);

	const auto position = physics.Camera->getGlobalPose().p;
	const auto distance = std::sqrt(position.x * position.x + position.y * position.y + position.z * position.z);

	REQUIRE(distance < 1.0f);
}
