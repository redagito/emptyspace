#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

enum FrustumSide
{
	RIGHT = 0,		// The RIGHT side of the frustum
	LEFT = 1,		// The LEFT	 side of the frustum
	BOTTOM = 2,		// The BOTTOM side of the frustum
	TOP = 3,		// The TOP side of the frustum
	BACK = 4,		// The BACK	side of the frustum
	FRONT = 5			// The FRONT side of the frustum
};

// Like above, instead of saying a number for the ABC and D of the plane, we
// want to be more descriptive.
enum PlaneData
{
	A = 0,				// The X value of the plane's normal
	B = 1,				// The Y value of the plane's normal
	C = 2,				// The Z value of the plane's normal
	D = 3				// The distance the plane is from the origin
};

class Frustum
{
public:
	void CalculateFrustum(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
	{
		const float* projection = glm::value_ptr(projectionMatrix);
		const float* view = glm::value_ptr(viewMatrix);

		float clip[16];

		// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
		// it will give us our clipping planes.  To combine 2 matrices, we multiply them.

		clip[0] = view[0] * projection[0] + view[1] * projection[4] + view[2] * projection[8] + view[3] * projection[12];
		clip[1] = view[0] * projection[1] + view[1] * projection[5] + view[2] * projection[9] + view[3] * projection[13];
		clip[2] = view[0] * projection[2] + view[1] * projection[6] + view[2] * projection[10] + view[3] * projection[14];
		clip[3] = view[0] * projection[3] + view[1] * projection[7] + view[2] * projection[11] + view[3] * projection[15];

		clip[4] = view[4] * projection[0] + view[5] * projection[4] + view[6] * projection[8] + view[7] * projection[12];
		clip[5] = view[4] * projection[1] + view[5] * projection[5] + view[6] * projection[9] + view[7] * projection[13];
		clip[6] = view[4] * projection[2] + view[5] * projection[6] + view[6] * projection[10] + view[7] * projection[14];
		clip[7] = view[4] * projection[3] + view[5] * projection[7] + view[6] * projection[11] + view[7] * projection[15];

		clip[8] = view[8] * projection[0] + view[9] * projection[4] + view[10] * projection[8] + view[11] * projection[12];
		clip[9] = view[8] * projection[1] + view[9] * projection[5] + view[10] * projection[9] + view[11] * projection[13];
		clip[10] = view[8] * projection[2] + view[9] * projection[6] + view[10] * projection[10] + view[11] * projection[14];
		clip[11] = view[8] * projection[3] + view[9] * projection[7] + view[10] * projection[11] + view[11] * projection[15];

		clip[12] = view[12] * projection[0] + view[13] * projection[4] + view[14] * projection[8] + view[15] * projection[12];
		clip[13] = view[12] * projection[1] + view[13] * projection[5] + view[14] * projection[9] + view[15] * projection[13];
		clip[14] = view[12] * projection[2] + view[13] * projection[6] + view[14] * projection[10] + view[15] * projection[14];
		clip[15] = view[12] * projection[3] + view[13] * projection[7] + view[14] * projection[11] + view[15] * projection[15];

		// Now we actually want to get the sides of the frustum.  To do this we take
		// the clipping planes we received above and extract the sides from them.

		// This will extract the RIGHT side of the frustum
		_frustum[RIGHT][A] = clip[3] - clip[0];
		_frustum[RIGHT][B] = clip[7] - clip[4];
		_frustum[RIGHT][C] = clip[11] - clip[8];
		_frustum[RIGHT][D] = clip[15] - clip[12];

		// Now that we have a normal (A,B,C) and a distance (D) to the plane,
		// we want to normalize that normal and distance.

		// Normalize the RIGHT side
		NormalizePlane(RIGHT);

		// This will extract the LEFT side of the frustum
		_frustum[LEFT][A] = clip[3] + clip[0];
		_frustum[LEFT][B] = clip[7] + clip[4];
		_frustum[LEFT][C] = clip[11] + clip[8];
		_frustum[LEFT][D] = clip[15] + clip[12];

		// Normalize the LEFT side
		NormalizePlane(LEFT);

		// This will extract the BOTTOM side of the frustum
		_frustum[BOTTOM][A] = clip[3] + clip[1];
		_frustum[BOTTOM][B] = clip[7] + clip[5];
		_frustum[BOTTOM][C] = clip[11] + clip[9];
		_frustum[BOTTOM][D] = clip[15] + clip[13];

		// Normalize the BOTTOM side
		NormalizePlane(BOTTOM);

		// This will extract the TOP side of the frustum
		_frustum[TOP][A] = clip[3] - clip[1];
		_frustum[TOP][B] = clip[7] - clip[5];
		_frustum[TOP][C] = clip[11] - clip[9];
		_frustum[TOP][D] = clip[15] - clip[13];

		// Normalize the TOP side
		NormalizePlane(TOP);

		// This will extract the BACK side of the frustum
		_frustum[BACK][A] = clip[3] - clip[2];
		_frustum[BACK][B] = clip[7] - clip[6];
		_frustum[BACK][C] = clip[11] - clip[10];
		_frustum[BACK][D] = clip[15] - clip[14];

		// Normalize the BACK side
		NormalizePlane(BACK);

		// This will extract the FRONT side of the frustum
		_frustum[FRONT][A] = clip[3] + clip[2];
		_frustum[FRONT][B] = clip[7] + clip[6];
		_frustum[FRONT][C] = clip[11] + clip[10];
		_frustum[FRONT][D] = clip[15] + clip[14];

		// Normalize the FRONT side
		NormalizePlane(FRONT);
	}

	bool PointInFrustum(float x, float y, float z)
	{
		for (int i = 0; i < 6; i++)
		{
			// Calculate the plane equation and check if the point is behind a side of the frustum
			if (_frustum[i][A] * x + _frustum[i][B] * y + _frustum[i][C] * z + _frustum[i][D] <= 0)
			{
				// The point was behind a side, so it ISN'T in the frustum
				return false;
			}
		}

		// The point was inside of the frustum (In front of ALL the sides of the frustum)
		return true;
	}

	bool SphereInFrustum(float x, float y, float z, float radius)
	{
		for (int i = 0; i < 6; i++)
		{
			// If the center of the sphere is farther away from the plane than the radius
			if (_frustum[i][A] * x + _frustum[i][B] * y + _frustum[i][C] * z + _frustum[i][D] <= -radius)
			{
				// The distance was greater than the radius so the sphere is outside of the frustum
				return false;
			}
		}

		// The sphere was inside of the frustum!
		return true;
	}

	bool CubeInFrustum(float x, float y, float z, float size)
	{
		for (int i = 0; i < 6; i++)
		{
			if (_frustum[i][A] * (x - size) + _frustum[i][B] * (y - size) + _frustum[i][C] * (z - size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x + size) + _frustum[i][B] * (y - size) + _frustum[i][C] * (z - size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x - size) + _frustum[i][B] * (y + size) + _frustum[i][C] * (z - size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x + size) + _frustum[i][B] * (y + size) + _frustum[i][C] * (z - size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x - size) + _frustum[i][B] * (y - size) + _frustum[i][C] * (z + size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x + size) + _frustum[i][B] * (y - size) + _frustum[i][C] * (z + size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x - size) + _frustum[i][B] * (y + size) + _frustum[i][C] * (z + size) + _frustum[i][D] > 0)
				continue;
			if (_frustum[i][A] * (x + size) + _frustum[i][B] * (y + size) + _frustum[i][C] * (z + size) + _frustum[i][D] > 0)
				continue;

			// If we get here, it isn't in the frustum
			return false;
		}

		return true;
	}

private:
	// Divides the plane by the length of its normal, so that the distance to
	// it can be read off the plane equation.
	void NormalizePlane(const int side)
	{
		const auto magnitude = static_cast<float>(sqrt(
			_frustum[side][A] * _frustum[side][A] +
			_frustum[side][B] * _frustum[side][B] +
			_frustum[side][C] * _frustum[side][C]));

		_frustum[side][A] /= magnitude;
		_frustum[side][B] /= magnitude;
		_frustum[side][C] /= magnitude;
		_frustum[side][D] /= magnitude;
	}

	float _frustum[6][4];
};
