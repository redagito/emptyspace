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
                               PxTolerancesScale(), true, nullptr);

    PxSceneDesc sceneDesc(_physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f);

    _dispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = _dispatcher;
    sceneDesc.filterShader  = PxDefaultSimulationFilterShader;

    _scene = _physics->createScene(sceneDesc);
    _material = _physics->createMaterial(0.5f, 0.5f, 0.6f);

    // World
    auto transform1 = PxTransform(PxVec3(0.0f));
    auto geometry1 = PxSphereGeometry(1.0f);
    World = PxCreateKinematic(*_physics, transform1, geometry1, *_material, 10.0f);
    World->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

    PxShape* shapes[1];
    World->getShapes(shapes, 1);
    shapes[0]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);

    // Camera
    auto transform2 = PxTransform(PxVec3(0.0f));  // Starting position
    auto geometry2 = PxSphereGeometry(1.0f);
    Camera = PxCreateDynamic(*_physics, transform2, geometry2, *_material, 10.0f);

    // Booster
    Booster = PxD6JointCreate(*_physics,
                              World,
                              PxTransform(PxVec3(0.0f)),
                              Camera,

                              // Attachment point of the "ship" 
                              PxTransform(PxVec3(0.0f, 0.0f, 0.0f)));

    // Affect position and orientation
    Booster->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eTWIST,  PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
    Booster->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);

    auto stiffness = 0.0f;
    auto damping = 5.0f;

    Booster->setDrive(PxD6Drive::eX, PxD6JointDrive(stiffness, damping, FLT_MAX, true));
    Booster->setDrive(PxD6Drive::eY, PxD6JointDrive(stiffness, damping, FLT_MAX, true));
    Booster->setDrive(PxD6Drive::eZ, PxD6JointDrive(stiffness, damping, FLT_MAX, true));
    Booster->setDrive(PxD6Drive::eTWIST, PxD6JointDrive(stiffness, damping, FLT_MAX, true));
    Booster->setDrive(PxD6Drive::eSWING, PxD6JointDrive(stiffness, damping, FLT_MAX, true));

    Booster->setDrivePosition(PxTransform(PxVec3(0.0f)));
    Booster->setDriveVelocity(PxVec3(0.0f), PxVec3(PxZero));

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
    
    // Move relative facing direction
    auto quat = Camera->getGlobalPose().q;
    auto xaxis = quat.getBasisVector0();
    auto yaxis = quat.getBasisVector1();
    auto zaxis = quat.getBasisVector2();

    if (direction == Direction::Forward)
    {
        LinearThrust -= zaxis * acceleration;
    }
    if (direction == Direction::Backward)
    {
        LinearThrust += zaxis * acceleration;
    }
    if (direction == Direction::Left)
    {
        LinearThrust -= xaxis * acceleration;
    }
    if (direction == Direction::Right)
    {
        LinearThrust += xaxis * acceleration;
    }
    if (direction == Direction::Up)
    {
        LinearThrust += yaxis * acceleration;
    }
    if (direction == Direction::Down)
    {
        LinearThrust -= yaxis * acceleration;
    }
    if (direction == Direction::RollCW)
    {
        AngularThrust.z += acceleration;
    }
    if (direction == Direction::RollCCW)
    {
        AngularThrust.z -= acceleration;
    }
    if (direction == Direction::Stop)
    {
        LinearThrust = PxVec3(0.0f);
        AngularThrust = PxVec3(0.0f);
    }

    std::clog << "LinearThrust (" << LinearThrust.x << ", " << LinearThrust.y << ", " << LinearThrust.z << ")" << std::endl;
    std::clog << "AngularThrust (" << AngularThrust.x << ", " << AngularThrust.y << ", " << AngularThrust.z << ")" << std::endl;
    Booster->setDriveVelocity(LinearThrust, AngularThrust);
}

void PhysicsScene::Tumble(const float x, const float y)
{
    auto sensitivity = 0.001f;

    AngularThrust.x += y * sensitivity;
    AngularThrust.y += x * sensitivity;

    Booster->setDriveVelocity(LinearThrust, AngularThrust);
}
