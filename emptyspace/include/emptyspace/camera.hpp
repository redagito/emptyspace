#pragma once
#include <glm/vec3.hpp>

class PhysicsScene;

struct Camera
{
    glm::vec3 Position;
    glm::vec3 Direction;

    static Camera FromPhysicsScene(const PhysicsScene& physicsScene)
    {
        const auto cameraPositionRaw = physicsScene.Camera->getGlobalPose().p;
        const auto cameraOrientation = physicsScene.Camera->getGlobalPose().q;
        const auto cameraDirectionRaw = cameraOrientation.getBasisVector2();
        const auto cameraPosition = glm::vec3(cameraPositionRaw.x, cameraPositionRaw.y, cameraPositionRaw.z);
        const auto cameraDirection = glm::vec3(cameraDirectionRaw.x, cameraDirectionRaw.y, cameraDirectionRaw.z);

        Camera camera;
        camera.Position = cameraPosition;
        camera.Direction = cameraDirection;
        return camera;
    }
};
