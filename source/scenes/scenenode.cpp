#include "scenes/scenenode.hpp"

#include <algorithm>

SceneNode::SceneNode(SceneNode* parent)
: _parent{ parent }
{
	_id = ++_nextId;
}

void SceneNode::AddChild(SceneNode* node)
{
	if (node == nullptr)
	{
		return;
	}

	_children.push_back(node);
}

void SceneNode::AddComponent(Component* component)
{
	if (component == nullptr)
	{
		return;
	}

	_components.push_back(component);
}

SceneNode* SceneNode::CreateChild()
{
	const auto child = new SceneNode(this);
	AddChild(child);
	return child;
}

void SceneNode::RemoveChild(SceneNode* node)
{
    if (const auto foundNode = std::find(_children.begin(), _children.end(), node); foundNode != _children.end())
	{
		_children.erase(foundNode);
	}
}