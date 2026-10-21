#pragma once

#include "types.hpp"

#include <PxConfig.h>
#include <PxPhysicsAPI.h>

#include <glm/glm.hpp>

#define PX_RELEASE(x) do { if (x) { (x)->release(); (x) = nullptr; } } while (false)

class PhysicsScene
{
public:
	// The longest step the simulation takes, a frame that took longer is
	// clamped to it.
	static constexpr f32 MaximumStep = 1.0f / 30.0f;

	// How fast the ship follows the turn rate it is commanded with, in units
	// per second.
	static constexpr f32 TurnResponse = 12.0f;

	PhysicsScene();
	~PhysicsScene();

	void Step(f32 deltaTime);

	// The speed the ship is commanded to have, in its own frame, in units per
	// second. The ship brakes towards it.
	void SetTargetSpeed(const glm::vec3& localSpeed);

	// The rate the ship is commanded to turn at, in its own frame, in radians
	// per second. The pitch is around the X axis, the yaw around the Y axis
	// and the roll around the Z axis.
	void SetTargetTurnRate(const glm::vec3& localRate);

	// Brakes the ship to a halt.
	void Stop();

	physx::PxD6Joint* Booster;
	physx::PxRigidDynamic* Camera;
	physx::PxRigidDynamic* World;

	// The speed the ship is commanded to have, in its own frame.
	glm::vec3 TargetSpeed{ 0.0f };

	// The rate the ship is commanded to turn at, in its own frame.
	glm::vec3 TargetTurnRate{ 0.0f };

	// The rate the ship actually turns at, in its own frame.
	glm::vec3 TurnRate{ 0.0f };

private:
	physx::PxDefaultAllocator      _allocator;
	physx::PxDefaultErrorCallback  _errorCallback;

	physx::PxFoundation* _foundation = nullptr;
	physx::PxPhysics* _physics = nullptr;

	physx::PxDefaultCpuDispatcher* _dispatcher = nullptr;
	physx::PxScene* _scene = nullptr;

	physx::PxMaterial* _material = nullptr;
};
