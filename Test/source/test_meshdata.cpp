#include <catch2/catch_test_macros.hpp>

#include "graphics/meshdata.hpp"

TEST_CASE("A model with normals is loaded with a normal vertex layout")
{
	const auto meshData = MeshData::FromFile("data/models/SM_ShipA_noWindshield.obj");

	REQUIRE(meshData->GetVertexType() == VertexType::PositionNormal);
}

TEST_CASE("A model without normals is loaded with a position only vertex layout")
{
	const auto meshData = MeshData::FromFile("data/models/SM_PointLight.obj");

	REQUIRE(meshData->GetVertexType() == VertexType::Position);
}

TEST_CASE("A loaded model is expanded into a triangle soup")
{
	const auto meshData = MeshData::FromFile("data/models/SM_PointLight.obj");

	REQUIRE(meshData->VertexCount() == 240);
	REQUIRE(meshData->IndexCount() == meshData->VertexCount());
}
