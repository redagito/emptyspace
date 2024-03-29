#pragma once

#include "types.hpp"

#include <PxConfig.h>
#include <PxPhysicsAPI.h>


#define PX_RELEASE(x) if(x) { x->release(); x = nullptr; }

enum class Direction
{
	Forward,
	Backward,
	Left,
	Right,
	Up,
	Down,
	RollCW,
	RollCCW,
	Stop
};

class PhysicsScene
{
public:
	PhysicsScene();
	~PhysicsScene();

	void Step(physx::PxReal deltaTime) const;
	void Boost(Direction direction, f32 acceleration);
	void Tumble(f32 x, f32 y);

	physx::PxD6Joint* Booster;
	physx::PxRigidDynamic* Camera;
	physx::PxRigidDynamic* World;

	physx::PxVec3               LinearThrust{ 0.0f, 0.0f, 0.0f };
	physx::PxVec3               AngularThrust{ 0.0f, 0.0f, 0.0f };

private:
	physx::PxDefaultAllocator      _allocator;
	physx::PxDefaultErrorCallback  _errorCallback;

	physx::PxFoundation* _foundation = nullptr;
	physx::PxPhysics* _physics = nullptr;

	physx::PxDefaultCpuDispatcher* _dispatcher = nullptr;
	physx::PxScene* _scene = nullptr;

	physx::PxMaterial* _material = nullptr;

	physx::PxPvd* _visualDebugger = nullptr;
};
