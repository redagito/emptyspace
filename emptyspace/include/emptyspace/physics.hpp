#pragma once

#include <ctype.h>
#include "PxPhysicsAPI.h"
#include <emptyspace/math/camera.hpp>

#define PX_RELEASE(x)   if(x)   { x->release(); x = NULL; }

enum class Direction
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class PhysicsScene {
public:
    PhysicsScene();
    ~PhysicsScene();

    void Step(physx::PxReal deltaTime);
    void Boost(Direction direction);
    glm::vec3 Fetch();

    physx::PxD6Joint*           Booster;
    physx::PxRigidDynamic*      Camera;
    physx::PxRigidDynamic*      World;

    physx::PxVec3               Thrust { 0.0f, 0.0f, 0.0f };

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
