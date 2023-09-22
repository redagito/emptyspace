#pragma once

#include <glm/mat4x4.hpp>
#include <vector>
#include "types.hpp"

class Material;

enum class Shape
{
    Cube = 0,
    Quad = 1,
    CubeInstanced = 2,
    Ship = 3
};

struct SceneObject
{
    glm::mat4 ModelViewProjection;
    glm::mat4 ModelViewProjectionPrevious;
    Shape ObjectShape;
    Material* ObjectMaterial;
    bool ExcludeFromMotionBlur;

    explicit SceneObject(
        const Shape shape = Shape::Cube,
        Material* material = nullptr,
        const bool excludeFromMotionBlur = false)
        : ModelViewProjection{},
        ModelViewProjectionPrevious{},
        ObjectShape{ shape },
        ObjectMaterial{ material },
        ExcludeFromMotionBlur{ excludeFromMotionBlur }
    {
    }
};

class Component;

class SceneNode
{
public:
    explicit SceneNode(SceneNode* parent);

    void AddChild(SceneNode* node);
    void AddComponent(Component* component);

    SceneNode* CreateChild();

    void RemoveChild(SceneNode* node);

    bool operator==(const SceneNode* other) const
    {
        return (other->_id == _id);
    }

private:
    inline static int _nextId{ 0 };

    u32 _id{};
    SceneNode* _parent{};
    std::vector<SceneNode*> _children{};
    std::vector<Component*> _components{};
};