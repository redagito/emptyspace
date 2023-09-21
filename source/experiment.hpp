#pragma once
// ReSharper disable CppInconsistentNaming
#include <emptyspace/types.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

class Model
{
    u32 vertex_count{0};
};

class ModelInstance
{
    Model model{};
    glm::mat4 model_matrix{};
    
};

class Buffer
{
public:
private:
    u32 id{0};
};



//class Scene
//{
//public:
//	Scene()
//	{
//		Root = new SceneNode(nullptr);
//	}
//	
//	void AddInstance(const Model& model, glm::mat4& model_matrix)
//	{
//		model_instances.emplace_back(model, model_matrix);
//	}
//
//	SceneNode* Root;
//private:
//	std::vector<ModelInstance> model_instances{};
//};
// ReSharper restore CppInconsistentNaming