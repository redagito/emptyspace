#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "math/ring.hpp"

TEST_CASE("Items on a ring are spread around it")
{
	REQUIRE(RingAngle(0, 4) == Catch::Approx(0.0f).margin(0.001f));
	REQUIRE(RingAngle(1, 4) == Catch::Approx(glm::half_pi<float>()).margin(0.001f));
	REQUIRE(RingAngle(2, 4) == Catch::Approx(glm::pi<float>()).margin(0.001f));
	REQUIRE(RingAngle(3, 4) == Catch::Approx(glm::half_pi<float>() * 3.0f).margin(0.001f));
}

TEST_CASE("The position on a ring is on the circle of that radius")
{
	REQUIRE(RingPosition(0, 4, 200.0f).x == Catch::Approx(0.0f).margin(0.01f));
	REQUIRE(RingPosition(0, 4, 200.0f).y == Catch::Approx(200.0f).margin(0.01f));

	REQUIRE(RingPosition(1, 4, 200.0f).x == Catch::Approx(200.0f).margin(0.01f));
	REQUIRE(RingPosition(1, 4, 200.0f).y == Catch::Approx(0.0f).margin(0.01f));

	REQUIRE(RingPosition(2, 4, 200.0f).x == Catch::Approx(0.0f).margin(0.01f));
	REQUIRE(RingPosition(2, 4, 200.0f).y == Catch::Approx(-200.0f).margin(0.01f));

	REQUIRE(RingPosition(3, 4, 200.0f).x == Catch::Approx(-200.0f).margin(0.01f));
	REQUIRE(RingPosition(3, 4, 200.0f).y == Catch::Approx(0.0f).margin(0.01f));
}
