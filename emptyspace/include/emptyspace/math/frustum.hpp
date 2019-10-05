#pragma once

#include <glm/mat4x4.hpp>
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

inline void NormalizePlane(float frustum[6][4], int side)
{
	// Here we calculate the magnitude of the normal to the plane (point A B C)
	// Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
	// To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
	const auto magnitude = static_cast<float>(sqrt(frustum[side][A] * frustum[side][A] + frustum[side][B] * frustum[side][B] + frustum[side][C] * frustum[side][C]));

	// Then we divide the plane's values by it's magnitude.
	// This makes it easier to work with.
	frustum[side][A] /= magnitude;
	frustum[side][B] /= magnitude;
	frustum[side][C] /= magnitude;
	frustum[side][D] /= magnitude;
}

class Frustum
{
public:
	void CalculateFrustum(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
	{
		float* proj = glm::value_ptr(projectionMatrix);
		float* modl = glm::value_ptr(viewMatrix);
		
		float   clip[16];

		// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
		// it will give us our clipping planes.  To combine 2 matrices, we multiply them.

		clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
		clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
		clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
		clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

		clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
		clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
		clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
		clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

		clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
		clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
		clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
		clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

		clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
		clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
		clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

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
		NormalizePlane(_frustum, RIGHT);

		// This will extract the LEFT side of the frustum
		_frustum[LEFT][A] = clip[3] + clip[0];
		_frustum[LEFT][B] = clip[7] + clip[4];
		_frustum[LEFT][C] = clip[11] + clip[8];
		_frustum[LEFT][D] = clip[15] + clip[12];

		// Normalize the LEFT side
		NormalizePlane(_frustum, LEFT);

		// This will extract the BOTTOM side of the frustum
		_frustum[BOTTOM][A] = clip[3] + clip[1];
		_frustum[BOTTOM][B] = clip[7] + clip[5];
		_frustum[BOTTOM][C] = clip[11] + clip[9];
		_frustum[BOTTOM][D] = clip[15] + clip[13];

		// Normalize the BOTTOM side
		NormalizePlane(_frustum, BOTTOM);

		// This will extract the TOP side of the frustum
		_frustum[TOP][A] = clip[3] - clip[1];
		_frustum[TOP][B] = clip[7] - clip[5];
		_frustum[TOP][C] = clip[11] - clip[9];
		_frustum[TOP][D] = clip[15] - clip[13];

		// Normalize the TOP side
		NormalizePlane(_frustum, TOP);

		// This will extract the BACK side of the frustum
		_frustum[BACK][A] = clip[3] - clip[2];
		_frustum[BACK][B] = clip[7] - clip[6];
		_frustum[BACK][C] = clip[11] - clip[10];
		_frustum[BACK][D] = clip[15] - clip[14];

		// Normalize the BACK side
		NormalizePlane(_frustum, BACK);

		// This will extract the FRONT side of the frustum
		_frustum[FRONT][A] = clip[3] + clip[2];
		_frustum[FRONT][B] = clip[7] + clip[6];
		_frustum[FRONT][C] = clip[11] + clip[10];
		_frustum[FRONT][D] = clip[15] + clip[14];

		// Normalize the FRONT side
		NormalizePlane(_frustum, FRONT);
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
	float _frustum[6][4];
};
