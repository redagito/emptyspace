#include <catch2/catch_test_macros.hpp>

#include "math/frustum.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace
{
	// A camera at the origin looking down -Z with a 90 degree field of view.
	glm::mat4 MakeProjection()
	{
		return glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 100.0f);
	}
}

TEST_CASE("Frustum accepts points in front of the camera and rejects the rest")
{
	Frustum frustum;
	frustum.CalculateFrustum(MakeProjection(), glm::mat4(1.0f));

	REQUIRE(frustum.PointInFrustum(0.0f, 0.0f, -10.0f));
	REQUIRE(frustum.PointInFrustum(5.0f, 5.0f, -50.0f));

	REQUIRE_FALSE(frustum.PointInFrustum(0.0f, 0.0f, 10.0f));
	REQUIRE_FALSE(frustum.PointInFrustum(0.0f, 0.0f, -0.5f));
	REQUIRE_FALSE(frustum.PointInFrustum(0.0f, 0.0f, -200.0f));
	REQUIRE_FALSE(frustum.PointInFrustum(80.0f, 0.0f, -10.0f));
}

TEST_CASE("Frustum accepts spheres that intersect it and rejects spheres that are entirely outside")
{
	Frustum frustum;
	frustum.CalculateFrustum(MakeProjection(), glm::mat4(1.0f));

	REQUIRE(frustum.SphereInFrustum(0.0f, 0.0f, -10.0f, 1.0f));
	REQUIRE(frustum.SphereInFrustum(0.0f, 0.0f, -10.0f, 50.0f));

	REQUIRE_FALSE(frustum.SphereInFrustum(0.0f, 0.0f, 10.0f, 1.0f));
	REQUIRE_FALSE(frustum.SphereInFrustum(0.0f, 0.0f, -200.0f, 1.0f));
}
