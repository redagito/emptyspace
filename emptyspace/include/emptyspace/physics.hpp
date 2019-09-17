#pragma once

#include "PxPhysicsAPI.h"

#define PX_RELEASE(x)   if(x)   { x->release(); x = NULL; }

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

    void Step(physx::PxReal deltaTime);
    void Boost(Direction direction);
    void Tumble(const float x, const float y);

    physx::PxD6Joint*           Booster;
    physx::PxRigidDynamic*      Camera;
    physx::PxRigidDynamic*      World;

    physx::PxVec3               LinearThrust { 0.0f, 0.0f, 0.0f };
    physx::PxVec3               AngularThrust { 0.0f, 0.0f, 0.0f };

private:
    physx::PxDefaultAllocator      _allocator;
    physx::PxDefaultErrorCallback  _errorCallback;

    physx::PxFoundation*           _foundation = nullptr;
    physx::PxPhysics*              _physics    = nullptr;

    physx::PxDefaultCpuDispatcher* _dispatcher = nullptr;
    physx::PxScene*                _scene      = nullptr;

    physx::PxMaterial*             _material   = nullptr;

    physx::PxPvd*                  _pvd        = nullptr;
};
