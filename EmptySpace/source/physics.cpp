#include "physics.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cmath>
#include <iostream>

using namespace physx;

PhysicsScene::PhysicsScene()
{
	std::clog << "PHYSX: Initialising.\n";

	_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocator, _errorCallback);
	_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, PxTolerancesScale(), true, nullptr);

	PxSceneDesc sceneDesc(_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f);

	_dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = _dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

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
	Booster->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
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
	Booster->setDriveVelocity(PxVec3(0.0f), PxZero);

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

void PhysicsScene::Step(const f32 deltaTime)
{
	if (deltaTime <= 0.0f)
	{
		return;
	}

	// A frame that took a long time must not move the simulation too far.
	const auto step = glm::clamp(deltaTime, 0.0f, MaximumStep);

	// The ship follows the turn rate it is commanded with.
	TurnRate += (TargetTurnRate - TurnRate) * (1.0f - std::exp(-TurnResponse * step));

	const auto pose = Camera->getGlobalPose();
	const auto orientation = glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z);

	// The drive of the joint takes the speed relative to the world and the
	// turn rate relative to the ship, which is the other way around from what
	// the ship is commanded in.
	const auto targetSpeed = orientation * TargetSpeed;
	const auto targetTurnRate = -TurnRate;

	Booster->setDriveVelocity(
		PxVec3(targetSpeed.x, targetSpeed.y, targetSpeed.z),
		PxVec3(targetTurnRate.x, targetTurnRate.y, targetTurnRate.z));

	_scene->simulate(step);
	_scene->fetchResults(true);
}

void PhysicsScene::SetTargetSpeed(const glm::vec3& localSpeed)
{
	TargetSpeed = localSpeed;
}

void PhysicsScene::SetTargetTurnRate(const glm::vec3& localRate)
{
	TargetTurnRate = localRate;
}

void PhysicsScene::Stop()
{
	TargetSpeed = glm::vec3(0.0f);
	TargetTurnRate = glm::vec3(0.0f);
}
