#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "physics.hpp"

struct Camera
{
	glm::vec3 Position;
	glm::vec3 Direction;
	glm::mat4 View;

	// The orientation of the ship, its nose points along the negative Z axis.
	glm::quat Orientation;

	static Camera FromPhysicsScene(const PhysicsScene& physicsScene)
	{
		const auto pose = physicsScene.Camera->getGlobalPose();

		const auto position = glm::vec3(pose.p.x, pose.p.y, pose.p.z);
		const auto orientation = glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z);
		const auto directionRaw = pose.q.getBasisVector2();
		const auto direction = glm::vec3(directionRaw.x, directionRaw.y, directionRaw.z);

		auto view = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
		view = glm::inverse(view);

		Camera camera;
		camera.Position = position;
		camera.Direction = direction;
		camera.View = view;
		camera.Orientation = orientation;
		return camera;
	}
};
