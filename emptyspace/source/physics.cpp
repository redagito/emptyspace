#include <vector>
#include <iostream>
#include <emptyspace/physics.hpp>
#include <glm/glm.hpp>

using namespace physx;

PhysicsScene::PhysicsScene()
{
    std::clog << "Initialising PhysX..\n";

    _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocator, _errorCallback);
    _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation,
                               PxTolerancesScale(), true, 0);

    PxSceneDesc sceneDesc(_physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f);

    _dispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = _dispatcher;
    sceneDesc.filterShader  = PxDefaultSimulationFilterShader;

    _scene = _physics->createScene(sceneDesc);
    _material = _physics->createMaterial(0.5f, 0.5f, 0.6f);

    // World
    auto transform1 = PxTransform(PxVec3(1.0f));
    auto geometry1 = PxSphereGeometry(1.0f);
    World = PxCreateKinematic(*_physics, transform1, geometry1, *_material, 10.0f);
    World->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

    PxShape* shapes[1];
    World->getShapes(shapes, 1);
    shapes[0]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);

    // Camera
    auto transform2 = PxTransform(PxVec3(0.0f, 0.0f, 5.0f));  // Starting position
    auto geometry2 = PxSphereGeometry(1.0f);
    Camera = PxCreateDynamic(*_physics, transform2, geometry2, *_material, 10.0f);

    // Booster
    Booster = PxD6JointCreate(*_physics,
                              World,
                              PxTransform(PxVec3(1.0f)),
                              Camera,
                              PxTransform(PxVec3(1.0f)));

    // Affect position, but not orientation
    Booster->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE);

    auto stiffness = 0.0f;
    auto damping = 5.0f;

    Booster->setDrive(PxD6Drive::eX, PxD6JointDrive(stiffness, damping, FLT_MAX, true));
    Booster->setDrive(PxD6Drive::eY, PxD6JointDrive(stiffness, damping, FLT_MAX, true));
    Booster->setDrive(PxD6Drive::eZ, PxD6JointDrive(stiffness, damping, FLT_MAX, true));

    Booster->setDrivePosition(PxTransform(PxVec3(1.0f)));
    Booster->setDriveVelocity(PxVec3(PxZero), PxVec3(PxZero));

    _scene->addActor(*World);
    _scene->addActor(*Camera);
}

PhysicsScene::~PhysicsScene()
{
    PX_RELEASE(_scene);
    PX_RELEASE(_dispatcher);
    PX_RELEASE(_physics);
    PX_RELEASE(_foundation);
}


void PhysicsScene::Step(PxReal deltaTime)
{
   _scene->simulate(deltaTime);
   _scene->fetchResults(true);
}


void PhysicsScene::Boost(Direction direction)
{
    auto acceleration = 0.05f;

    if (direction == Direction::Forward)
    {
        Thrust.z -= acceleration;
    }
    if (direction == Direction::Backward)
    {
        Thrust.z += acceleration;
    }
    if (direction == Direction::Left)
    {
        Thrust.x -= acceleration;
    }
    if (direction == Direction::Right)
    {
        Thrust.x += acceleration;
    }
    if (direction == Direction::Up)
    {
        Thrust.y += acceleration;
    }
    if (direction == Direction::Down)
    {
        Thrust.y -= acceleration;
    }

    std::clog << "Thrust (" << Thrust.x << ", " << Thrust.y << ", " << Thrust.z << ")" << std::endl;
    Booster->setDriveVelocity(Thrust, PxVec3(PxZero));
}


glm::vec3 PhysicsScene::Fetch()
{
    auto pose = Camera->getGlobalPose();
    return glm::vec3(pose.p.x, pose.p.y, pose.p.z);
}
